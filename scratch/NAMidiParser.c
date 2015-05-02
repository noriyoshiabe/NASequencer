#include "NAMidiParser.h"
#include "Expression.h"
#include "Parser.h"
#include "Lexer.h"

#include <stdlib.h>
#include <CoreFoundation/CoreFoundation.h>

#define PANIC(fmt, __VA_ARGS__) do { printf(fmt, __VA_ARGS__); abort(); } while (0)

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

    _NAMidiParserParseAST(self, expression);
    ExpressionDestroy(expression);

    return true;
}

static bool _NAMidiParserParseDSL(NAMidiParser *self, const char *filepath, Expression **expression)
{
    bool ret = false;
    void *scanner;
    *expression = NULL;

    ParseLocation location;
    location.filepath = filepath;

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        self->callbacks->onError(self->receiver, filepath, 0, 0, ParseErrorFileNotFound);
        return ret;
    }

    if (yylex_init_extra(&location, &scanner)) {
        self->callbacks->onError(self->receiver, filepath, 0, 0, ParseErrorInitError);
        goto ERROR_1;
    }

    YY_BUFFER_STATE state = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);

    if (yyparse(scanner, expression)) {
        self->callbacks->onError(self->receiver, location.filepath, location.firstLine, location.firstColumn, ParseErrorSyntaxError);

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
    ParseLocation *location = yyget_extra(scanner);
    location->firstLine = yylloc->first_line;
    location->firstColumn = yylloc->first_column;
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

typedef struct _Context {
    NAMidiParser *parser;
    TimeTable *timeTable;
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

static Context *ContextCreate(NAMidiParser *parser)
{
    Context *ret = calloc(1, sizeof(Context));
    ret->parser = parser;
    ret->timeTable = TimeTableCreate();
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

static TimeSign *ContextGetTimeSignByTick(Context *self, int32_t tick)
{
    CFIndex count = CFArrayGetCount(self->timeTable->timeSignList);
    if (0 == count) {
        TimeSign *timeSign = TimeSignCreate(0, 4, 4);
        self->parser->callbacks->onParseTime(self->parser->receiver, 0, 4, 4);
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

static int32_t ContextGetResolution(Context *self)
{
    if (0 == self->timeTable->resolution) {
        // TODO
        //self->parser->callbacks->onParseResolution(self->parser->receiver, 480);
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

static int32_t ContextGetGlobalTick(Context *self)
{
    if (self->parent) {
        return self->tick + ContextGetGlobalTick(self->parent);
    }

    return self->tick;
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

    for (Expression *expr = expression; expr; expr = expr->right) {
        if (!parseExpression(expr, context, NULL)) {
            ContextDestroy(context);
            return false;
        }
    }

    ContextDestroy(context);
    return true;
}

#define CALLBACK_ERROR(context, expression, error, ...) \
    context->parser->callbacks->onError( \
            context->parser->receiver, \
            expression->location.filepath, \
            expression->location.firstLine, \
            expression->location.firstColumn, \
            error, \
            __VA_ARGS__)

#define isPowerOf2(x) ((x != 0) && ((x & (x - 1)) == 0))

static bool parseInteger(Expression *expression, Context *context, void *value)
{
    *((int32_t *)value) = expression->v.i;
    return true;
}

static int baseNoteNo(Context *context, char c)
{
    const struct {
        char c;
        int noteNo;
    } noteMap[] = {
        {'c', 12},
        {'d', 14},
        {'e', 16},
        {'f', 17},
        {'g', 19},
        {'a', 21},
        {'b', 23},
    };

    for (int i = 0; i < sizeof(noteMap)/sizeof(noteMap[0]); ++i) {
        if (noteMap[i].c == c) {
            return noteMap[i].noteNo;
        }
    }

    return -1;
}

static bool canSharp(Context *context, char c)
{
    switch (c) {
    case 'c':
    case 'd':
    case 'f':
    case 'g':
    case 'a':
        return true;
    }

    return false;
}

static bool canFlat(Context *context, char c)
{
    switch (c) {
    case 'd':
    case 'e':
    case 'g':
    case 'a':
    case 'b':
        return true;
    }

    return false;
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
            noteNo = baseNoteNo(context, c) + 12 * context->octave;
            noteChar = c;
            break;

        case '+':
            if (canSharp(context, noteChar)) {
                noteNo++;
            }
            else {
                CALLBACK_ERROR(context, expression, ParseErrorNoteIllegalSharp, noteChar);
                return false;
            }
            break;

        case '-':
            if (canFlat(context, noteChar)) {
                noteNo--;
            }
            else {
                CALLBACK_ERROR(context, expression, ParseErrorNoteIllegalFlat, noteChar);
                return false;
            }
            break;

        case '^':
            noteNo += 12;
            if (127 < noteNo) {
                CALLBACK_ERROR(context, expression, ParseErrorNoteIllegalOctaveUp, noteNo - 12);
                return false;
            }
            break;

        case '_':
            noteNo -= 12;
            if (0 > noteNo) {
                CALLBACK_ERROR(context, expression, ParseErrorNoteIllegalOctaveUp, noteNo + 12);
                return false;
            }
            break;

        default:
            PANIC("Unexpected character in note string. c=%c\n", noteString[i]);
            break;
        }
    }

    uint32_t tick = ContextGetGlobalTick(context);
    uint32_t gatetime = 0 < context->gatetime ? context->gatetime : ContextGetStep(context);

    context->parser->callbacks->onParseNote(
            context->parser,
            tick,
            context->channel,
            noteNo,
            context->velocity,
            gatetime);

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
    TimeSign *timeSign = NULL;

    for (Expression *expr = expression->left; expr; expr = expr->right) {
        switch (expr->type) {
        case ExpressionTypeTimeSign:
            if (!parseExpression(expr, context, &timeSign)) {
                if (timeSign) {
                    TimeSignDestroy(timeSign);
                }
                return false;
            }
            context->step = ContextGetResolution(context) * 4 / timeSign->denominator * timeSign->numerator;
            break;

        case ExpressionTypeInteger:
            parseExpression(expr, context, &tuplet);

            if (isValidTuplet(tuplet)) {
                context->step = context->step * 2 / tuplet;
            }
            else {
                CALLBACK_ERROR(context, expression, ParseErrorNoteInvalidTaplet, tuplet);
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

    TimeSignDestroy(timeSign);
    return true;
}

static bool parseTimeSign(Expression *expression, Context *context, void *value)
{
    TimeSign **out = (TimeSign **)value;

    int32_t numerator;
    int32_t denominator;

    parseExpression(expression->left, context, &numerator);
    parseExpression(expression->left->right, context, &denominator);

    if (1 > numerator || 1 > denominator || !isPowerOf2(denominator)) {
        CALLBACK_ERROR(context, expression, ParseErrorNoteInvalidTimeSign, numerator, denominator);
        return false;
    }

    *out = TimeSignCreate(0, numerator, denominator);
    return true;
}



static void __attribute__((constructor)) initializeTable()
{
    functionTable[ExpressionTypeInteger] = parseInteger;
    functionTable[ExpressionTypeNote] = parseNote;
    functionTable[ExpressionTypeQuantize] = parseQuantize;
    functionTable[ExpressionTypeTimeSign] = parseTimeSign;
}
