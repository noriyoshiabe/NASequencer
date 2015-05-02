#include "NAMidiParser.h"
#include "Expression.h"
#include "Parser.h"
#include "Lexer.h"

#include <stdlib.h>
#include <CoreFoundation/CoreFoundation.h>

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


static bool (*functionTable[ExpressionTypeSize])(Expression *expression, Context *context, void *value) = {NULL};

static bool parseExpression(Expression *expression, Context *context, void *value)
{
#if 0
    printf("Attempt to parse expression [%s]\n", ExpressionType2String(expression->type));
#endif

    bool (*function)(Expression *, Context *, void *) = functionTable[expression->type];
    if (!function) {
        printf("Parse function is not found. type=%s\n", ExpressionType2String(expression->type));
        abort();
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
