#include "NAMidiParser.h"
#include "Expression.h"
#include "YYContext.h"
#include "Parser.h"
#include "Lexer.h"
#include "TimeTable.h"
#include "TimeTable_internal.h"
#include "NoteTable.h"

#include <stdlib.h>
#include <sys/param.h>
#include <alloca.h>
#include <CoreFoundation/CoreFoundation.h>

#define PANIC(...) do { printf(__VA_ARGS__); abort(); } while (0)

struct _NAMidiParser {
    NAMidiParserCallbacks *callbacks;
    void *receiver;
};

int yyparse(void *scanner, Expression **expression);
static bool _NAMidiParserParseDSL(NAMidiParser *self, const char *filepath, Expression **expression);
static bool _NAMidiParserParseAST(NAMidiParser *self, Expression *expression);

NAMidiParser *NAMidiParserCreate(NAMidiParserCallbacks *callbacks, void *receiver)
{
    NAMidiParser *self = calloc(1, sizeof(NAMidiParser));
    self->callbacks = callbacks;
    self->receiver = receiver;
    return self;
}

void NAMidiParserDestroy(NAMidiParser *self)
{
    free(self);
}

bool NAMidiParserExecuteParse(NAMidiParser *self, const char *filepath)
{
    Expression *expression;

    if (!_NAMidiParserParseDSL(self, filepath, &expression)) {
        return false;
    }

#if 0
    ExpressionDump(expression);
#endif

    bool success = _NAMidiParserParseAST(self, expression);
    ExpressionDestroy(expression);

    return success;
}

static bool _NAMidiParserParseDSL(NAMidiParser *self, const char *filepath, Expression **expression)
{
    bool ret = false;
    void *scanner;
    *expression = NULL;

    YYContext context = {0};
    context.location.filepath = filepath;

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        self->callbacks->onError(self->receiver, filepath, 0, 0, ParseErrorFileNotFound, NULL);
        return ret;
    }

    if (yylex_init_extra(&context, &scanner)) {
        self->callbacks->onError(self->receiver, filepath, 0, 0, ParseErrorInitError, NULL);
        goto ERROR_1;
    }

    YY_BUFFER_STATE state = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);

    if (yyparse(scanner, expression)) {
        self->callbacks->onError(self->receiver, context.location.filepath, context.location.firstLine, context.location.firstColumn, ParseErrorSyntaxError, NULL);

        if (*expression) {
            ExpressionDestroy(*expression);
            *expression = NULL;
        }
        goto ERROR_2;
    }

    ret = true;

ERROR_2:
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
ERROR_1:
    fclose(fp);

    return ret;
}

int yyerror(YYLTYPE *yylloc, void *scanner, Expression **expression, const char *message)
{
    YYContext *context = yyget_extra(scanner);
    context->location.firstLine = yylloc->first_line;
    context->location.firstColumn = yylloc->first_column;
    return 0;
}

typedef struct _Context {
    NAMidiParser *parser;
    TimeTable *timeTable;
    NoteTableKeySign key;
    int32_t tick;
    int32_t step;
    int32_t channel;
    int32_t velocity;
    int32_t gatetime;
    int32_t octave;
    int32_t length;

    int32_t largestTick;
    Expression *offsetLength;
    int32_t blockOffset;
    int32_t blockLength;

    CFMutableDictionaryRef patterns;
    CFMutableSetRef expandingPattens;

    struct _Context *parent;
} Context;

static bool ContextOnParseNote(Context *self, int32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity, uint32_t gatetime);
static bool ContextOnParseMarker(Context *self, int32_t tick, const char *text);

static bool parseExpression(Expression *expression, Context *context, void *value);

static Context *ContextCreate(NAMidiParser *parser, TimeTable *timeTable)
{
    Context *ret = calloc(1, sizeof(Context));
    ret->parser = parser;
    ret->timeTable = timeTable;
    ret->key = NoteTableKeySignCMajor;
    ret->channel = 1;
    ret->octave = 4;
    ret->velocity = 100;
    ret->patterns = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, NULL);
    ret->expandingPattens = CFSetCreateMutable(NULL, 0, &kCFTypeSetCallBacks);
    ret->blockLength = -1;
    return ret;
}

static void ContextDestroy(Context *self)
{
    TimeTableRelease(self->timeTable);
    CFRelease(self->patterns);
    CFRelease(self->expandingPattens);
    free(self);
}

static Context *ContextCreateFromContext(Context *from)
{
    Context *ret = ContextCreate(from->parser, TimeTableCreateFromTimeTable(from->timeTable, from->tick));
    ret->parent = from;

    ret->key = from->key;
    ret->step = from->step;
    ret->channel = from->channel;
    ret->velocity = from->velocity;
    ret->gatetime = from->gatetime;
    ret->octave = from->octave;

    ret->patterns = CFDictionaryCreateMutableCopy(NULL, 0, from->patterns);
    ret->expandingPattens = CFSetCreateMutableCopy(NULL, 0, from->expandingPattens);

    return ret;
}

static int32_t ContextGetStep(Context *self)
{
    if (0 == self->step) {
        self->step = TimeTableResolution(self->timeTable);
    }

    return self->step;
}

static void ContextForwardTick(Context *self)
{
    self->tick += ContextGetStep(self);
    self->largestTick = MAX(self->largestTick, self->tick);
}

static bool ContextCalcOffsetLength(Context *self)
{
    for (Expression *expr = self->offsetLength; expr; expr = expr->next) {
        switch (expr->type) {
        case ExpressionTypeOffset:
            if (!parseExpression(expr, self, &self->blockOffset)) {
                return false;
            }
            break;
        case ExpressionTypeLength:
            if (!parseExpression(expr, self, &self->blockLength)) {
                return false;
            }
            break;
        default:
            break;
        }
    }

    return true;
}

static void ContextForwardTickWithContext(Context *self, Context *local)
{
    self->tick += -1 != local->blockLength ? local->blockLength : local->largestTick;
    self->largestTick = MAX(self->largestTick, self->tick);
}

static uint32_t ContextGetTickInParent(Context *self, uint32_t tick)
{
    return tick + self->parent->tick;
}

#define isInsideOffsetLength(context, tick) (0 <= tick && (-1 == context->blockLength || tick < context->blockLength))

static bool ContextOnParseNote(Context *self, int32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity, uint32_t gatetime)
{
    tick -= self->blockOffset;
    if (!isInsideOffsetLength(self, tick)) {
        return true;
    }

    gatetime = MIN(gatetime, self->blockLength - tick);

    if (self->parent) {
        ContextOnParseNote(self->parent, ContextGetTickInParent(self, tick), channel, noteNo, velocity, gatetime);
    }
    else {
        self->parser->callbacks->onParseNote(self->parser->receiver, tick, channel, noteNo, velocity, gatetime);
    }

    return true;
}

static bool ContextOnParseMarker(Context *self, int32_t tick, const char *text)
{
    tick -= self->blockOffset;
    if (!isInsideOffsetLength(self, tick)) {
        return true;
    }

    if (self->parent) {
        ContextOnParseMarker(self->parent, ContextGetTickInParent(self, tick), text);
    }
    else {
        self->parser->callbacks->onParseMarker(self->parser->receiver, tick, text);
    }

    return true;
}


static bool (*functionTable[ExpressionTypeSize])(Expression *expression, Context *context, void *value) = {NULL};

static bool parseExpression(Expression *expression, Context *context, void *value)
{
#if 0
    printf("Attempt to parse expression [%s]\n", ExpressionType2String(expression->type));
#endif

    bool (*function)(Expression *, Context *, void *) = functionTable[expression->type];
    if (!function) {
        PANIC("Parse function is not found. type=%s\n", ExpressionType2String(expression->type));
    }
    return function(expression, context, value);
}

static bool _NAMidiParserParseAST(NAMidiParser *self, Expression *expression)
{
    Context *context = ContextCreate(self, TimeTableCreate());

    for (Expression *expr = expression; expr; expr = expr->next) {
        if (!parseExpression(expr, context, NULL)) {
            ContextDestroy(context);
            return false;
        }
    }

    context->parser->callbacks->onParseResolution(context->parser->receiver, TimeTableResolution(context->timeTable));

    size_t count, i;

    count = TimeTableGetTimeSignCount(context->timeTable);
    TimeEvent *timeEvents = alloca(count * sizeof(TimeEvent *));
    TimeTableGetTimeSignValues(context->timeTable, &timeEvents);
    for (i = 0; i < count; ++i) {
        context->parser->callbacks->onParseTime(context->parser->receiver, timeEvents[i].tick, timeEvents[i].timeSign.numerator, timeEvents[i].timeSign.denominator);
    }

    count = TimeTableGetTempoCount(context->timeTable);
    TempoEvent *tempoEvents = alloca(count * sizeof(TempoEvent *));
    TimeTableGetTempoValues(context->timeTable, &tempoEvents);
    for (i = 0; i < count; ++i) {
        context->parser->callbacks->onParseTempo(context->parser->receiver, tempoEvents[i].tick, tempoEvents[i].tempo);
    }

    TimeTableSetLength(context->timeTable, context->largestTick);

    context->parser->callbacks->onFinish(context->parser->receiver, context->timeTable);
    ContextDestroy(context);
    return true;
}

#define CALLBACK_ERROR(context, expression, error, info) \
    context->parser->callbacks->onError( \
            context->parser->receiver, \
            expression->location.filepath, \
            expression->location.firstLine, \
            expression->location.firstColumn, \
            error, \
            info)

#define isPowerOf2(x) ((x != 0) && ((x & (x - 1)) == 0))
#define isValidRange(v, from, to) (from <= v && v <= to)

static bool parseInteger(Expression *expression, Context *context, void *value)
{
    *((int32_t *)value) = expression->v.i;
    return true;
}

static bool parseFloat(Expression *expression, Context *context, void *value)
{
    *((float *)value) = expression->v.f;
    return true;
}

static bool parseResolution(Expression *expression, Context *context, void *value)
{
    if (context->parent || 0 < context->tick) {
        CALLBACK_ERROR(context, expression, ParseErrorNoteIllegalResolution, NULL);
        return false;
    }

    if (!TimeTableSetResolution(context->timeTable, expression->v.i)) {
        CALLBACK_ERROR(context, expression, ParseErrorNoteIllegalResolution, &expression->v.i);
        return false;
    }

    return true;
}

typedef struct _Note {
    uint32_t tick;
    uint8_t noteNo;
    uint8_t channel;
    uint8_t velocity;
    uint32_t gatetime;
    struct _Note *next;
} Note;

static Note *NoteCreate(uint32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity, uint32_t gatetime)
{
    Note *ret = calloc(1, sizeof(Note));
    ret->tick = tick;
    ret->channel = channel;
    ret->noteNo = noteNo;
    ret->velocity = velocity;
    ret->gatetime = gatetime;
    return ret;
}

static bool parseNoteBlock(Expression *expression, Context *context, void *value)
{
    Note *noteList = NULL;
    for (Expression *expr = expression->child; expr; expr = expr->next) {
        if (ExpressionTypeNoteList == expr->type) {
            if (!parseExpression(expr, context, &noteList)) {
                return false;
            }
        }
        else if (ExpressionTypeTie == expr->type) {
            parseExpression(expr, context, noteList);
        }
        else {
            PANIC("Unexpected ExpressionType type=%s\n", ExpressionType2String(expr->type));
        }
    }

    bool success = true;
    for (Note *note = noteList; note; note = note->next) {
        success = ContextOnParseNote(context, note->tick, note->channel, note->noteNo, note->velocity, note->gatetime);
        if (!success) {
            break;
        }
    }

    for (Note *note = noteList; note; ) {
        Note *willFree = note;
        note = note->next;
        free(willFree);
    }

    return success;
}

static bool parseNoteList(Expression *expression, Context *context, void *value)
{
    Note *head = NULL;
    Note *last = NULL;
    for (Expression *expr = expression->child; expr; expr = expr->next) {
        Note *note;
        if (!parseExpression(expr, context, &note)) {
            return false;
        }

        if (!head) {
            head = note;
        }

        if (last) {
            last->next = note;
        }

        last = note;
    }

    *((Note **)value) = head;
    ContextForwardTick(context);
    return true;
}

static bool parseNote(Expression *expression, Context *context, void *value)
{
    const char *noteString = expression->v.s;
    int length = strlen(noteString);

    int noteNo = -1;
    char noteChar = -1;

    for (int i = 0; i < length; ++i) {
        char c = tolower(noteString[i]);
        switch (c) {
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'a':
        case 'b':
            noteNo = NoteTableGetBaseNoteNo(context->key, c) + 12 * context->octave;
            noteChar = c;
            break;

        case '+':
            noteNo++;
            if (127 < noteNo) {
                CALLBACK_ERROR(context, expression, ParseErrorNoteIllegalSharp, &noteChar);
                return false;
            }
            break;

        case '-':
            noteNo--;
            if (0 > noteNo) {
                CALLBACK_ERROR(context, expression, ParseErrorNoteIllegalFlat, &noteChar);
                return false;
            }
            break;

        case '=':
            noteNo += NoteTableGetNaturalDiff(context->key, noteChar);
            if (noteNo < 0 || 127 < noteNo) {
                CALLBACK_ERROR(context, expression, ParseErrorNoteIllegalNatural, &noteChar);
                return false;
            }
            break;

        case '^':
            noteNo += 12;
            if (127 < noteNo) {
                CALLBACK_ERROR(context, expression, ParseErrorNoteIllegalOctaveUp, &noteChar);
                return false;
            }
            break;

        case '_':
            noteNo -= 12;
            if (0 > noteNo) {
                CALLBACK_ERROR(context, expression, ParseErrorNoteIllegalOctaveDown, &noteChar);
                return false;
            }
            break;

        default:
            PANIC("Unexpected character in note string. c=%c\n", noteString[i]);
            break;
        }
    }

    uint32_t gatetime = 0 < context->gatetime ? context->gatetime : ContextGetStep(context);
    *((Note **)value) = NoteCreate(context->tick, context->channel, noteNo, context->velocity, gatetime);
    return true;
}

static bool parseTie(Expression *expression, Context *context, void *value)
{
    for (Note *note = value; note; note = note->next) {
        note->gatetime += ContextGetStep(context);
    }

    ContextForwardTick(context);
    return true;
}

static bool parseRest(Expression *expression, Context *context, void *value)
{
    ContextForwardTick(context);
    return true;
}

static bool isValidTuplet(int32_t tuplet)
{
    switch (tuplet) {
    case 3:
    case 5:
    case 7:
    case 9:
        return true;
    }

    return false;
}

static bool parseQuantize(Expression *expression, Context *context, void *value)
{
    int32_t tuplet;
    int32_t step;

    for (Expression *expr = expression->child; expr; expr = expr->next) {
        switch (expr->type) {
        case ExpressionTypeTimeSign:
            if (!parseExpression(expr, context, &step)) {
                return false;
            }
            context->step = step;
            break;

        case ExpressionTypeInteger:
            parseExpression(expr, context, &tuplet);

            if (isValidTuplet(tuplet)) {
                context->step = context->step * 2 / tuplet;
            }
            else {
                CALLBACK_ERROR(context, expression, ParseErrorQuantizeInvalidTaplet, &tuplet);
                return false;
            }
            break;

        case ExpressionTypeDot:
            context->step += context->step / 2;
            break;
        
        default:
            PANIC("Unexpected ExpressionType type=%s\n", ExpressionType2String(expr->type));
            break;
        }
    }

    return true;
}

static bool parseOctaveShift(Expression *expression, Context *context, void *value)
{
    switch (expression->v.s[0]) {
    case '<':
        context->octave--;
        if (context->octave < -2) {
            CALLBACK_ERROR(context, expression, ParseErrorIllegalOctaveShift, &context->octave);
            return false;
        }
        return true;
    case '>':
        context->octave++;
        if (8 < context->octave) {
            CALLBACK_ERROR(context, expression, ParseErrorIllegalOctaveShift, &context->octave);
            return false;
        }
        return true;
    default:
        return false;
    }
}

static bool parseKey(Expression *expression, Context *context, void *value)
{
    const char *keyString = expression->v.s;

    char keyChar = keyString[0];
    bool sharp = NULL != strchr(keyString, '+');
    bool flat = NULL != strchr(keyString, '-');
    bool major = NULL == strstr(keyString, "min");

    NoteTableKeySign key = NoteTableGetKeySign(keyChar, sharp, flat, major);
    if (NoteTableKeySignInvalid == key) {
        CALLBACK_ERROR(context, expression, ParseErrorInvalidKeySign, keyString);
        return false;
    }

    context->key = key;
    return true;
}

static bool parseTime(Expression *expression, Context *context, void *value)
{
    TimeSign timeSign;
    if (!parseExpression(expression->child, context, &timeSign)) {
        return false;
    }

    TimeTableAddTimeSign(context->timeTable, context->tick, timeSign);
    return true;
}

static bool parseTimeSign(Expression *expression, Context *context, void *value)
{
    int32_t numerator;
    int32_t denominator;

    parseExpression(expression->child, context, &numerator);
    parseExpression(expression->child->next, context, &denominator);

    if (1 > numerator || 1 > denominator || !isPowerOf2(denominator)) {
        TimeSign timeSign = {numerator, denominator};
        CALLBACK_ERROR(context, expression, ParseErrorInvalidTimeSign, &timeSign);
        return false;
    }

    if (ExpressionTypeTime == expression->parent->type) {
        ((TimeSign *)value)->numerator = numerator;
        ((TimeSign *)value)->denominator = denominator;
    }
    else {
        *((int32_t *)value) = TimeTableResolution(context->timeTable) * 4 / denominator * numerator;
    }
    
    return true;
}

static bool parseTempo(Expression *expression, Context *context, void *value)
{
    float tempo;
    int _int;

    switch (expression->child->type) {
    case ExpressionTypeInteger:
        parseExpression(expression->child, context, &_int);
        tempo = _int;
        break;
    case ExpressionTypeFloat:
        parseExpression(expression->child, context, &tempo);
        break;
    default:
        PANIC("Unexpected ExpressionType type=%s\n", ExpressionType2String(expression->child->type));
        break;
    }

    if (!isValidRange(tempo, 30.0, 300.0)) {
        CALLBACK_ERROR(context, expression, ParseErrorInvalidTempo, &tempo);
        return false;
    }

    TimeTableAddTempo(context->timeTable, context->tick, tempo);
    return true;
}

static bool parseMarker(Expression *expression, Context *context, void *value)
{
    return ContextOnParseMarker(context, context->tick, expression->v.s);
}

static bool parseChannel(Expression *expression, Context *context, void *value)
{
    if (!isValidRange(expression->v.i, 1, 16)) {
        CALLBACK_ERROR(context, expression, ParseErrorInvalidChannel, &expression->v.i);
        return false;
    }

    context->channel = expression->v.i;
    return true;
}

static bool parseVelocity(Expression *expression, Context *context, void *value)
{
    if (!isValidRange(expression->v.i, 0, 127)) {
        CALLBACK_ERROR(context, expression, ParseErrorInvalidVelociy, &expression->v.i);
        return false;
    }

    context->velocity = expression->v.i;
    return true;
}

static bool parseGatetime(Expression *expression, Context *context, void *value)
{
    if (!isValidRange(expression->v.i, 1, 65535)) {
        CALLBACK_ERROR(context, expression, ParseErrorInvalidGatetime, &expression->v.i);
        return false;
    }

    context->gatetime = expression->v.i;
    return true;
}

static bool parseGatetimeAuto(Expression *expression, Context *context, void *value)
{
    context->gatetime = 0;
    return true;
}

static bool parseOctave(Expression *expression, Context *context, void *value)
{
    if (!isValidRange(expression->v.i, -1, 9)) {
        CALLBACK_ERROR(context, expression, ParseErrorInvalidOctave, &expression->v.i);
        return false;
    }

    context->octave = expression->v.i;
    return true;
}

static bool parseLocation(Expression *expression, Context *context, void *value)
{
    if (!parseExpression(expression->child, context, &context->tick)) {
        return false;
    }

    if (0 > context->tick) {
        CALLBACK_ERROR(context, expression, ParseErrorInvalidLocation, NULL);
        return false;
    }

    return true;
}

static bool parseMeasure(Expression *expression, Context *context, void *value)
{
    if (1 > expression->v.i) {
        CALLBACK_ERROR(context, expression, ParseErrorInvalidMeasure, NULL);
        return false;
    }

    *((int32_t *)value) = TimeTableTickByMeasure(context->timeTable, expression->v.i);
    return true;
}

static bool parsePlus(Expression *expression, Context *context, void *value)
{
    int32_t left, right;
    if (!parseExpression(expression->child, context, &left)) {
        return false;
    }
    if (!parseExpression(expression->child->next, context, &right)) {
        return false;
    }

    *((int32_t *)value) = left + right;
    return true;
}

static bool parseMinus(Expression *expression, Context *context, void *value)
{
    int32_t left, right;
    if (!parseExpression(expression->child, context, &left)) {
        return false;
    }
    if (!parseExpression(expression->child->next, context, &right)) {
        return false;
    }

    *((int32_t *)value) = left - right;
    return true;
}

static bool parseRepeat(Expression *expression, Context *context, void *value)
{
    for (int i = 0; i < expression->v.i; ++i) {
        if (!parseExpression(expression->child, context, NULL)) {
            return false;
        }
    }
    return true;
}

static bool parseBlock(Expression *expression, Context *context, void *value)
{
    Context *local = ContextCreateFromContext(context);
    if (value) {
        local->offsetLength = value;
        if (!ContextCalcOffsetLength(local)) {
            return false;
        }
    }

    for (Expression *expr = expression->child; expr; expr = expr->next) {
        if (!parseExpression(expr, local, NULL)) {
            return false;
        }
    }

    ContextForwardTickWithContext(context, local);
    return true;
}

static bool parseParallel(Expression *expression, Context *context, void *value)
{
    int32_t tick = context->tick;
    int32_t resultTick = tick;

    for (Expression *expr = expression->child; expr; expr = expr->next) {
        if (!parseExpression(expr, context, NULL)) {
            return false;
        }
        resultTick = MAX(resultTick, context->tick);
        context->tick = tick;
    }

    context->tick = resultTick;
    return true;
}

static bool parsePatternDefine(Expression *expression, Context *context, void *value)
{
    CFStringRef identifier = CFStringCreateWithCString(NULL, expression->child->v.s, kCFStringEncodingUTF8);
    CFDictionarySetValue(context->patterns, identifier, expression->child->next);
    CFRelease(identifier);
    return true;
}

static bool parsePatternExpand(Expression *expression, Context *context, void *value)
{
    bool ret = false;
    CFStringRef identifier = CFStringCreateWithCString(NULL, expression->child->v.s, kCFStringEncodingUTF8);
    CFIndex size = CFStringGetLength(identifier) + 1;
    char *cstring = alloca(size);
    CFStringGetCString(identifier, cstring, size, kCFStringEncodingUTF8);
    
    if (CFSetContainsValue(context->expandingPattens, identifier)) {
        CALLBACK_ERROR(context, expression, ParseErrorPatternCircularReference, cstring);
        goto ERROR;
    }

    Expression *pattern = (Expression *)CFDictionaryGetValue(context->patterns, identifier);

    if (!pattern) {
        CALLBACK_ERROR(context, expression, ParseErrorPatternMissing, cstring);
        goto ERROR;
    }

    CFSetSetValue(context->expandingPattens, identifier);
    ret = parseExpression(pattern, context, expression->child->next);
    CFSetRemoveValue(context->expandingPattens, identifier);

ERROR:
    CFRelease(identifier);
    return ret;
}

static bool parseOffset(Expression *expression, Context *context, void *value)
{
    int32_t tick;

    if (!parseExpression(expression->child, context, &tick)) {
        return false;
    }

    if (0 > tick) {
        CALLBACK_ERROR(context, expression, ParseErrorInvalidOffset, NULL);
        return false;
    }

    *((int32_t *)value) = tick;
    return true;
}

static bool parseLength(Expression *expression, Context *context, void *value)
{
    int32_t tick;

    if (!parseExpression(expression->child, context, &tick)) {
        return false;
    }

    if (0 > tick) {
        CALLBACK_ERROR(context, expression, ParseErrorInvalidLength, NULL);
        return false;
    }

    *((int32_t *)value) = tick;
    return true;
}

static bool parseLengthValue(Expression *expression, Context *context, void *value)
{
    *((int32_t *)value) = TimeTableTickByMeasure(context->timeTable, expression->v.i + 1);
    return true;
}


static void __attribute__((constructor)) initializeTable()
{
    functionTable[ExpressionTypeInteger] = parseInteger;
    functionTable[ExpressionTypeFloat] = parseFloat;
    functionTable[ExpressionTypeResolution] = parseResolution;
    functionTable[ExpressionTypeNoteBlock] = parseNoteBlock;
    functionTable[ExpressionTypeNoteList] = parseNoteList;
    functionTable[ExpressionTypeNote] = parseNote;
    functionTable[ExpressionTypeTie] = parseTie;
    functionTable[ExpressionTypeRest] = parseRest;
    functionTable[ExpressionTypeQuantize] = parseQuantize;
    functionTable[ExpressionTypeOctaveShift] = parseOctaveShift;
    functionTable[ExpressionTypeKey] = parseKey;
    functionTable[ExpressionTypeTime] = parseTime;
    functionTable[ExpressionTypeTimeSign] = parseTimeSign;
    functionTable[ExpressionTypeTempo] = parseTempo;
    functionTable[ExpressionTypeMarker] = parseMarker;
    functionTable[ExpressionTypeChannel] = parseChannel;
    functionTable[ExpressionTypeVelocity] = parseVelocity;
    functionTable[ExpressionTypeGatetime] = parseGatetime;
    functionTable[ExpressionTypeGatetimeAuto] = parseGatetimeAuto;
    functionTable[ExpressionTypeOctave] = parseOctave;
    functionTable[ExpressionTypeLocation] = parseLocation;
    functionTable[ExpressionTypeMeasure] = parseMeasure;
    functionTable[ExpressionTypePlus] = parsePlus;
    functionTable[ExpressionTypeMinus] = parseMinus;
    functionTable[ExpressionTypeRepeat] = parseRepeat;
    functionTable[ExpressionTypeBlock] = parseBlock;
    functionTable[ExpressionTypeParallel] = parseParallel;
    functionTable[ExpressionTypePatternDefine] = parsePatternDefine;
    functionTable[ExpressionTypePatternExpand] = parsePatternExpand;
    functionTable[ExpressionTypeOffset] = parseOffset;
    functionTable[ExpressionTypeLength] = parseLength;
    functionTable[ExpressionTypeLengthValue] = parseLengthValue;
}