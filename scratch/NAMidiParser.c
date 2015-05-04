#include "NAMidiParser.h"
#include "Expression.h"
#include "YYContext.h"
#include "Parser.h"
#include "Lexer.h"
#include "NoteTable.h"

#include <stdlib.h>
#include <sys/param.h>
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

#if 1
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
        self->callbacks->onError(self->receiver, filepath, 0, 0, ParseErrorFileNotFound);
        return ret;
    }

    if (yylex_init_extra(&context, &scanner)) {
        self->callbacks->onError(self->receiver, filepath, 0, 0, ParseErrorInitError);
        goto ERROR_1;
    }

    YY_BUFFER_STATE state = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);

    if (yyparse(scanner, expression)) {
        self->callbacks->onError(self->receiver, context.location.filepath, context.location.firstLine, context.location.firstColumn, ParseErrorSyntaxError);

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


typedef struct _TimeSign {
    int32_t tick;
    int16_t numerator;
    int16_t denominator;
} TimeSign;

static TimeSign *TimeSignCreate(int32_t tick, int16_t numerator, int16_t denominator)
{
    TimeSign *ret = calloc(1, sizeof(TimeSign));
    ret->tick = tick;
    ret->numerator = numerator;
    ret->denominator = denominator;
    return ret;
}

static TimeSign *TimeSignCreateWithTimeSign(int32_t tick, const TimeSign *from)
{
    TimeSign *ret = calloc(1, sizeof(TimeSign));
    ret->tick = tick;
    ret->numerator = from->numerator;
    ret->denominator = from->denominator;
    return ret;
}

static void TimeSignDestroy(TimeSign *self)
{
    free(self);
}

typedef struct _TimeTable {
    int32_t resolution;
    CFMutableArrayRef timeSignList;
} TimeTable;

static void TimeSignReleaseCallbak(CFAllocatorRef allocator, const void *value)
{
    TimeSignDestroy((TimeSign *)value);
}
const CFArrayCallBacks TimeTableTimeSignListCallbacks = {0, NULL, TimeSignReleaseCallbak, NULL, NULL};

static TimeTable *TimeTableCreate()
{
    TimeTable *ret = calloc(1, sizeof(TimeTable));
    ret->timeSignList = CFArrayCreateMutable(NULL, 0, &TimeTableTimeSignListCallbacks);
    return ret;
}

static void TimeTableDestroy(TimeTable *self)
{
    CFRelease(self->timeSignList);
    free(self);
}

CFComparisonResult TimeTableTimeSignComparator(const void *val1, const void *val2, void *context)
{
    return ((TimeSign *)val1)->tick - ((TimeSign *)val2)->tick;
}

static void TimeTableAddTimeSign(TimeTable *self, const TimeSign *timeSign)
{
    CFArrayAppendValue(self->timeSignList, timeSign);
    CFArraySortValues(self->timeSignList, CFRangeMake(0, CFArrayGetCount(self->timeSignList)), TimeTableTimeSignComparator, NULL);
}

static int32_t TimeTableGetTickByMeasure(TimeTable *self, int32_t measure)
{
    int32_t offsetTick = 0;
    int32_t tickPerMeasure = self->resolution * 4;

    measure -= 1;

    CFIndex count = CFArrayGetCount(self->timeSignList);
    for (int i = 0; i < count; ++i) {
        const TimeSign *timeSign = CFArrayGetValueAtIndex(self->timeSignList, i);

        int32_t tick = tickPerMeasure * measure + offsetTick;
        if (tick < timeSign->tick) {
            break;
        }

        measure -= timeSign->tick / tickPerMeasure;
        tickPerMeasure = self->resolution * 4 / timeSign->denominator * timeSign->numerator;
        offsetTick = timeSign->tick;
    }

    return tickPerMeasure * measure + offsetTick;
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

    struct _Context *parent;
} Context;

static int32_t ContextGetResolution(Context *self);
static TimeSign *ContextGetTimeSignByTick(Context *self, int32_t tick);
static int32_t ContextGetTickByMeasure(Context *self, int32_t measure);

static void ContextOnParseResolution(Context *self, uint32_t resolution);
static void ContextOnParseNote(Context *self, uint32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity, uint32_t gatetime);
static void ContextOnParseTime(Context *self, uint32_t tick, uint8_t numerator, uint8_t denominator);
static void ContextOnParseTempo(Context *self, uint32_t tick, float tempo);
static void ContextOnParseMarker(Context *self, uint32_t tick, const char *text);

static Context *ContextCreate(NAMidiParser *parser)
{
    Context *ret = calloc(1, sizeof(Context));
    ret->parser = parser;
    ret->timeTable = TimeTableCreate();
    ret->key = NoteTableKeySignCMajor;
    ret->channel = 1;
    ret->octave = 4;
    ret->velocity = 100;
    return ret;
}

static void ContextDestroy(Context *self)
{
    TimeTableDestroy(self->timeTable);
    free(self);
}

static Context *ContextCreateFromContext(Context *from)
{
    Context *ret = ContextCreate(from->parser);

    ret->parent = from;

    ret->key = from->key;
    ret->step = from->step;
    ret->channel = from->channel;
    ret->velocity = from->velocity;
    ret->gatetime = from->gatetime;
    ret->octave = from->octave;

    ret->timeTable->resolution = ContextGetResolution(from);

    TimeSign *timeSign = ContextGetTimeSignByTick(from, from->tick);
    TimeTableAddTimeSign(ret->timeTable, TimeSignCreateWithTimeSign(0, timeSign));

    return ret;
}

static void ContextAddTimeSign(Context *self, const TimeSign *timeSign)
{
    TimeTableAddTimeSign(self->timeTable, timeSign);
}

static TimeSign *ContextGetTimeSignByTick(Context *self, int32_t tick)
{
    CFIndex count = CFArrayGetCount(self->timeTable->timeSignList);
    if (0 == count) {
        TimeSign *timeSign = TimeSignCreate(0, 4, 4);
        ContextOnParseTime(self, 0, 4, 4);
        TimeTableAddTimeSign(self->timeTable, timeSign);
        return timeSign;
    }

    TimeSign *target = NULL;
    for (CFIndex i = 0; i < count; ++i) {
        TimeSign *test = (TimeSign *)CFArrayGetValueAtIndex(self->timeTable->timeSignList, i);
        if (tick < test->tick) {
            break;
        }

        target = test;
    }

    return target;
}

static int32_t ContextGetTickByMeasure(Context *self, int32_t measure)
{
    CFIndex count = CFArrayGetCount(self->timeTable->timeSignList);
    if (0 == count) {
        TimeSign *timeSign = TimeSignCreate(0, 4, 4);
        ContextOnParseTime(self, 0, 4, 4);
        TimeTableAddTimeSign(self->timeTable, timeSign);
    }

    ContextGetResolution(self);

    return TimeTableGetTickByMeasure(self->timeTable, measure);
}

static int32_t ContextGetResolution(Context *self)
{
    if (0 == self->timeTable->resolution) {
        ContextOnParseResolution(self, 480);
        self->timeTable->resolution = 480;
    }

    return self->timeTable->resolution;
}

static int32_t ContextGetStep(Context *self)
{
    if (0 == self->step) {
        self->step = ContextGetResolution(self);
    }

    return self->step;
}

static uint32_t ContextGetTickInParent(Context *self, uint32_t tick)
{
    return tick + self->parent->tick;
}

static void ContextOnParseResolution(Context *self, uint32_t resolution)
{
    if (self->parent) {
        ContextOnParseResolution(self->parent, resolution);
        return;
    }

    // TODO
    //self->parser->callbacks->onParseResolution(self->parser->receiver, resolution);
}

static void ContextOnParseNote(Context *self, uint32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity, uint32_t gatetime)
{
    if (self->parent) {
        ContextOnParseNote(self->parent, ContextGetTickInParent(self, tick), channel, noteNo, velocity, gatetime);
    }
    else {
        self->parser->callbacks->onParseNote(self->parser, tick, channel, noteNo, velocity, gatetime);
    }
}

static void ContextOnParseTime(Context *self, uint32_t tick, uint8_t numerator, uint8_t denominator)
{
    if (self->parent) {
        return;
    }

    self->parser->callbacks->onParseTime(self->parser->receiver, tick, numerator, denominator);
}

static void ContextOnParseTempo(Context *self, uint32_t tick, float tempo)
{
    if (self->parent) {
        ContextOnParseTempo(self->parent, ContextGetTickInParent(self, tick), tempo);
    }
    else {
        self->parser->callbacks->onParseTempo(self->parser->receiver, tick, tempo);
    }
}

static void ContextOnParseMarker(Context *self, uint32_t tick, const char *text)
{
    if (self->parent) {
        ContextOnParseMarker(self->parent, ContextGetTickInParent(self, tick), text);
    }
    else {
        self->parser->callbacks->onParseMarker(self->parser->receiver, tick, text);
    }
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
    Context *context = ContextCreate(self);

    for (Expression *expr = expression; expr; expr = expr->next) {
        if (!parseExpression(expr, context, NULL)) {
            ContextDestroy(context);
            return false;
        }
    }

    context->parser->callbacks->onFinish(context->parser->receiver, context->length);
    ContextDestroy(context);
    return true;
}

#define CALLBACK_ERROR(context, expression, ...) \
    context->parser->callbacks->onError( \
            context->parser->receiver, \
            expression->location.filepath, \
            expression->location.firstLine, \
            expression->location.firstColumn, \
            __VA_ARGS__)

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

    for (Note *note = noteList; note; ) {
        ContextOnParseNote(context, note->tick, note->channel, note->noteNo, note->velocity, note->gatetime);

        Note *willFree = note;
        note = note->next;
        free(willFree);
    }

    return true;
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
    context->tick += ContextGetStep(context);
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
                CALLBACK_ERROR(context, expression, ParseErrorNoteIllegalSharp);
                return false;
            }
            break;

        case '-':
            noteNo--;
            if (0 > noteNo) {
                CALLBACK_ERROR(context, expression, ParseErrorNoteIllegalFlat);
                return false;
            }
            break;

        case '=':
            noteNo += NoteTableGetNaturalDiff(context->key, noteChar);
            if (noteNo < 0 || 127 < noteNo) {
                CALLBACK_ERROR(context, expression, ParseErrorNoteIllegalNatural);
                return false;
            }
            break;

        case '^':
            noteNo += 12;
            if (127 < noteNo) {
                CALLBACK_ERROR(context, expression, ParseErrorNoteIllegalOctaveUp);
                return false;
            }
            break;

        case '_':
            noteNo -= 12;
            if (0 > noteNo) {
                CALLBACK_ERROR(context, expression, ParseErrorNoteIllegalOctaveDown);
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

    context->tick += ContextGetStep(context);
    return true;
}

static bool parseRest(Expression *expression, Context *context, void *value)
{
    context->tick += ContextGetStep(context);
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
                CALLBACK_ERROR(context, expression, ParseErrorQuantizeInvalidTaplet, tuplet);
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
            CALLBACK_ERROR(context, expression, ParseErrorIllegalOctaveShift, context->octave);
            return false;
        }
        return true;
    case '>':
        context->octave++;
        if (8 < context->octave) {
            CALLBACK_ERROR(context, expression, ParseErrorIllegalOctaveShift, context->octave);
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
        CALLBACK_ERROR(context, expression, ParseErrorInvalidKeySign);
        return false;
    }

    context->key = key;
    return true;
}

static bool parseTime(Expression *expression, Context *context, void *value)
{
    TimeSign *timeSign = NULL;
    if (!parseExpression(expression->child, context, &timeSign)) {
        return false;
    }
    
    ContextAddTimeSign(context, timeSign);
    ContextOnParseTime(context, context->tick, timeSign->numerator, timeSign->denominator);
    return true;
}

static bool parseTimeSign(Expression *expression, Context *context, void *value)
{
    int32_t numerator;
    int32_t denominator;

    parseExpression(expression->child, context, &numerator);
    parseExpression(expression->child->next, context, &denominator);

    if (1 > numerator || 1 > denominator || !isPowerOf2(denominator)) {
        CALLBACK_ERROR(context, expression, ParseErrorInvalidTimeSign, numerator, denominator);
        return false;
    }

    if (ExpressionTypeTime == expression->parent->type) {
        *((TimeSign **)value) = TimeSignCreate(context->tick, numerator, denominator);
    }
    else {
        *((int32_t *)value) = ContextGetResolution(context) * 4 / denominator * numerator;
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
        CALLBACK_ERROR(context, expression, ParseErrorInvalidTempo, tempo);
        return false;
    }

    ContextOnParseTempo(context, context->tick, tempo);
    return true;
}

static bool parseMarker(Expression *expression, Context *context, void *value)
{
    ContextOnParseMarker(context, context->tick, expression->v.s);
    return true;
}

static bool parseChannel(Expression *expression, Context *context, void *value)
{
    if (!isValidRange(expression->v.i, 1, 16)) {
        CALLBACK_ERROR(context, expression, ParseErrorInvalidChannel, expression->v.i);
        return false;
    }

    context->channel = expression->v.i;
    return true;
}

static bool parseVelocity(Expression *expression, Context *context, void *value)
{
    if (!isValidRange(expression->v.i, 0, 127)) {
        CALLBACK_ERROR(context, expression, ParseErrorInvalidVelociy, expression->v.i);
        return false;
    }

    context->velocity = expression->v.i;
    return true;
}

static bool parseGatetime(Expression *expression, Context *context, void *value)
{
    if (!isValidRange(expression->v.i, 1, 65535)) {
        CALLBACK_ERROR(context, expression, ParseErrorInvalidGatetime, expression->v.i);
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
        CALLBACK_ERROR(context, expression, ParseErrorInvalidOctave, expression->v.i);
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
        CALLBACK_ERROR(context, expression, ParseErrorInvalidLocation);
        return false;
    }

    return true;
}

static bool parseMeasure(Expression *expression, Context *context, void *value)
{
    if (1 > expression->v.i) {
        CALLBACK_ERROR(context, expression, ParseErrorInvalidMeasure);
        return false;
    }

    *((int32_t *)value) = ContextGetTickByMeasure(context, expression->v.i);
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
    for (Expression *expr = expression->child; expr; expr = expr->next) {
        if (!parseExpression(expr, local, NULL)) {
            return false;
        }
    }
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


static void __attribute__((constructor)) initializeTable()
{
    functionTable[ExpressionTypeInteger] = parseInteger;
    functionTable[ExpressionTypeFloat] = parseFloat;
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
}
