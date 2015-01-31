#include "ASTParser.h"
#include "Parser.h"
#include <NACFHelper.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
 * TODO error handling
 */

typedef struct _Context {
    NAType _;
    Sequence *sequence;
    uint32_t tick;
    int32_t channel;
    int32_t velocity;
    int32_t gatetime;
    int32_t octave;
    CFMutableArrayRef events;
    TimeTable *timeTable;
} Context;

static void *__ContextInit(void *_self, ...)
{
    Context *self = _self;
    
    va_list ap;
    va_start(ap, _self);
    self->sequence = NARetain(va_arg(ap, Sequence *));
    va_end(ap);
    
    self->events = CFArrayCreateMutable(NULL, 0, NACFArrayCallBacks);
    self->timeTable = NATypeNew(TimeTable);
    return self;
}

static void ContextAddEvent(Context *context, void *event)
{
    CFArrayAppendValue(context->events, event);
}

static void __ContextDestroy(void *_self)
{
    Context *self = _self;

    CFRelease(self->events);
    NARelease(self->sequence);
    NARelease(self->timeTable);
}

static void *__ContextDescription(const void *_self)
{
    const Context *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<Context: timeTable=%@ events=%@>"), self->timeTable, self->events);
}

NADeclareVtbl(Context, NAType,
        __ContextInit,
        __ContextDestroy,
        NULL,
        NULL,
        NULL,
        NULL,
        __ContextDescription,
        );

NADeclareClass(Context, NAType);

typedef struct _NoteBlockContext {
    NAType _;
    int32_t tick;
    int32_t step;
    CFMutableArrayRef events;
} NoteBlockContext;

static void *__NoteBlockContextInit(void *_self, ...)
{
    NoteBlockContext *self = _self;
    
    va_list ap;
    va_start(ap, _self);
    self->tick = va_arg(ap, int32_t);
    self->step = va_arg(ap, int32_t);
    va_end(ap);
    
    self->events = CFArrayCreateMutable(NULL, 0, NACFArrayCallBacks);
    return self;
}

static void NoteBlockContextAddEvent(NoteBlockContext *context, void *event)
{
    CFArrayAppendValue(context->events, event);
}

static void __NoteBlockContextDestroy(void *_self)
{
    NoteBlockContext *self = _self;
    CFRelease(self->events);
}

static void *__NoteBlockContextDescription(const void *_self)
{
    const NoteBlockContext *self = _self;
    return (void *)CFStringCreateWithFormat(NULL, NULL, CFSTR("<NoteBlockContext: step=%d events=%@>"), self->step, self->events);
}

NADeclareVtbl(NoteBlockContext, NAType,
        __NoteBlockContextInit,
        __NoteBlockContextDestroy,
        NULL,
        NULL,
        NULL,
        NULL,
        __NoteBlockContextDescription,
        );

NADeclareClass(NoteBlockContext, NAType);


#define TABLE_SIZE (TOKEN_END - TOKEN_BEGIN)
#define IDX(type) (type - TOKEN_BEGIN - 1)
#define SET_ERROR(error, _kind, _expression, _message) (error->kind = _kind, error->expression = _expression, error->message = _message ? _message : "")

static bool (*dispatchTable[TABLE_SIZE])(Expression *, Context *, void *, ASTParserError *) = {NULL};

static bool parseExpression(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    bool (*function)(Expression *, Context *, void *, ASTParserError *)= dispatchTable[IDX(expression->tokenType)];
    if (!function) {
        fprintf(stderr, "Dispatch function is not found. tokenType=%s\n", tokenType2String(expression->tokenType));
        abort();
    }
    return function(expression, context, value, error);
}

Sequence *ASTParserParseExpression(Expression *expression, const char *filepath, ASTParserError *error)
{
    Sequence *sequence = NATypeNew(Sequence);
    Context *context = NATypeNew(Context, sequence);

    error->filepath = filepath;

    Expression *expr = expression;

    do {
        if (!parseExpression(expr, context, NULL, error)) {
            NARelease(sequence);
            sequence = NULL;
            goto ERROR;
        }
    } while ((expr = expr->right));

    SequenceAddEvents(sequence, context->events);

ERROR:
    NARelease(context);

    return sequence;
}

static bool __dispatch__INTEGER(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    *((int32_t *)value) = expression->v.i;
    return true;
}

static bool __dispatch__FLOAT(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    *((float *)value) = expression->v.f;
    return true;
}

static bool __dispatch__STRING(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    *((char **)value) = strdup(expression->v.s);
    return true;
}


static int noteNoString2Int(Context *context, const char *noteNoString)
{
    const struct {
        char *name;
        int baseKey;
    } noteMap[] = {
        {"C", 24},
        {"C#", 25}, {"DB", 25},
        {"D", 26},
        {"D#", 27}, {"EB", 27},
        {"E", 28},
        {"F", 29},
        {"F#", 30}, {"GB", 30},
        {"G", 31},
        {"G#", 32}, {"AB", 32},
        {"A", 33},
        {"A#", 34}, {"BB", 34},
        {"B", 35},
    };

    char octave[4] = {0};
    char *pOctave = octave;

    char noteNo[8];
    strcpy(noteNo, noteNoString);
    char *pNoteNo = noteNo;
    while (*pNoteNo) {
        *pNoteNo = toupper(*pNoteNo);
        if (*pNoteNo == '-' || *pNoteNo == '+' || isdigit(*pNoteNo)) {
            *pOctave = *pNoteNo;
            *pNoteNo = '\0';
            *(++pOctave) = '\0';
        }
        ++pNoteNo;
    }

    int baseKey = -1;
    for (int i = 0; i < sizeof(noteMap)/sizeof(noteMap[0]); ++i) {
        if (0 == strcmp(noteMap[i].name, noteNo)) {
            baseKey = noteMap[i].baseKey;
            break;
        }
    }

    if (-1 == baseKey) {
        return -1;
    }
    else {
        context->octave = '\0' != octave[0] ? atoi(octave) : context->octave;
        return baseKey + 12 * context->octave;
    }
}

static bool __dispatch__NOTE_NO(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    NoteBlockContext *nbContext = value;

    NoteEvent *noteEvent = NATypeNew(NoteEvent, nbContext->tick);
    noteEvent->channel = context->channel;
    noteEvent->noteNo = noteNoString2Int(context, expression->v.s);
    noteEvent->velocity = context->velocity;
    
    int32_t gatetime = 0 <= context->gatetime ? context->gatetime : context->gatetime + nbContext->step;
    noteEvent->gatetime = 0 < gatetime ? gatetime : 0;

    for (Expression *expr = expression->left; expr; expr = expr->right) {
        parseExpression(expr, context, noteEvent, error);
    }

    NoteBlockContextAddEvent(nbContext, noteEvent);
    NARelease(noteEvent);

    return true;
}

static bool __dispatch__LOCATION(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    if (strrchr(expression->v.s, ':')) {
        char buf[16];
        int32_t numbers[3];
        int count = 0;
        char *str = buf;
        char *token;

        strcpy(buf, expression->v.s);

        while ((token = strtok_r(str, ":", &str))) {
            numbers[count] = atoi(token);
            ++count;
            str = NULL;
        }

        switch (count) {
        case 2:
            *((int32_t *)value) = TimeTableLocation2Tick(context->timeTable, 1, numbers[0], numbers[1]);
            break;
        case 3:
            *((int32_t *)value) = TimeTableLocation2Tick(context->timeTable, numbers[0], numbers[1], numbers[2]);
            break;
        }
    }
    else if (('b' == expression->v.s[0])) {
        int32_t beat = atoi(&expression->v.s[1]);
        *((int32_t *)value) = TimeTableLocation2Tick(context->timeTable, 1, beat, 0);
    }
    else if (('m' == expression->v.s[0])) {
        int32_t measure = atoi(&expression->v.s[1]);
        *((int32_t *)value) = TimeTableLocation2Tick(context->timeTable, measure, 1, 0);
    }

    return true;
}

static bool __dispatch__MB_LENGTH(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__MB_LENGTH()\n");
    return true;
}


static bool __dispatch__RESOLUTION(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    if (0 != context->sequence->resolution) {
        SET_ERROR(error, ASTPARSER_RESOLUTION_REDEFINED, expression, "resolution cannot be defined twice.");
        return false;
    }

    parseExpression(expression->left, context, &context->sequence->resolution, error);
    context->timeTable->resolution = context->sequence->resolution;

    return true;
}

static bool __dispatch__TITLE(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    if (0 != context->sequence->title) {
        SET_ERROR(error, ASTPARSER_TITLE_REDEFINED, expression, "title cannot be defined twice.");
        return false;
    }

    return parseExpression(expression->left, context, &context->sequence->title, error);
}

static bool __dispatch__TIME(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    TimeEvent *event = NATypeNew(TimeEvent, context->tick);

    Expression *expr = expression->left;

    do {
        parseExpression(expr, context, event, error);
    } while ((expr = expr->right));

    TimeTableAddTimeEvent(context->timeTable, event);
    ContextAddEvent(context, event);
    NARelease(event);

    return true;
}

static bool __dispatch__TEMPO(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    TempoEvent *event = NATypeNew(TempoEvent, context->tick);

    Expression *expr = expression->left;

    do {
        if (FLOAT == expr->tokenType) {
            parseExpression(expr, context, &event->tempo, error);
        }
        else {
            parseExpression(expr, context, event, error);
        }
    } while ((expr = expr->right));

    TimeTableAddTempoEvent(context->timeTable, event);
    ContextAddEvent(context, event);
    NARelease(event);

    return true;
}

static bool __dispatch__MARKER(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    MarkerEvent *event = NATypeNew(MarkerEvent, context->tick);

    Expression *expr = expression->left;

    do {
        if (STRING == expr->tokenType) {
            parseExpression(expr, context, &event->text, error);
        }
        else {
            parseExpression(expr, context, event, error);
        }
    } while ((expr = expr->right));

    ContextAddEvent(context, event);
    NARelease(event);

    return true;
}

static bool __dispatch__CHANNEL(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    return parseExpression(expression->left, context, &context->channel, error);
}

static bool __dispatch__VELOCITY(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    int32_t val; 
    parseExpression(expression->left, context, &val, error);

    int tokenType = expression->parent ? expression->parent->tokenType : -1;
    switch (tokenType) {
    case NOTE_NO:
        ((NoteEvent *)value)->velocity = val;
        break;
    default:
        context->velocity = val;
        break;
    }

    return true;
}

static bool __dispatch__GATETIME(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    int32_t val; 
    parseExpression(expression->left, context, &val, error);

    int tokenType = expression->parent ? expression->parent->tokenType : -1;
    switch (tokenType) {
    case NOTE_NO:
        ((NoteEvent *)value)->gatetime = val;
        break;
    default:
        context->gatetime = val;
        break;
    }

    return true;
}

static bool __dispatch__NOTE(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    int32_t step = -1;

    Expression *noteBlockExpr = NULL;
    Expression *expr = expression->left;

    do {
        switch (expr->tokenType) {
        case FROM:
            parseExpression(expr, context, &context->tick, error);
            break;
        case STEP:
            parseExpression(expr, context, &step, error);
            break;
        case NOTE_BLOCK:
            noteBlockExpr = expr;
            break;
        }
    } while ((expr = expr->right));

    return parseExpression(noteBlockExpr, context, &step, error);
}


static bool __dispatch__STEP(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    return parseExpression(expression->left, context, value, error);
}

static bool __dispatch__FROM(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    switch (expression->parent->tokenType) {
    case NOTE:
        return parseExpression(expression->left, context, value, error);
    default:
        return parseExpression(expression->left, context, &((MidiEvent *)value)->tick, error);
    }
}

static bool __dispatch__TO(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__TO()\n");
    return true;
}

static bool __dispatch__REPLACE(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__REPLACE()\n");
    return true;
}

static bool __dispatch__MIX(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__MIX()\n");
    return true;
}

static bool __dispatch__OFFSET(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__OFFSET()\n");
    return true;
}

static bool __dispatch__LENGTH(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__LENGTH()\n");
    return true;
}


static bool __dispatch__REST(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    return true;
}

static bool __dispatch__TIE(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    NoteBlockContext *nbContext = value;
    int32_t tick = nbContext->tick - nbContext->step;
    CFIndex count = CFArrayGetCount(nbContext->events);
    for (int i = count - 1; 0 <= count; --i) {
        NoteEvent *event = (NoteEvent *)CFArrayGetValueAtIndex(nbContext->events, i);
        if (tick != event->_.tick) {
            break;
        }
        event->gatetime += nbContext->step;
    }
    return true;
}


static bool __dispatch__PLUS(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__PLUS()\n");
    return true;
}

static bool __dispatch__MINUS(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__MINUS()\n");
    return true;
}

static bool __dispatch__DIVISION(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__DIVISION()\n");
    return true;
}

static bool __dispatch__MULTIPLY(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__MULTIPLY()\n");
    return true;
}

static bool __dispatch__ASSIGN(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__ASSIGN()\n");
    return true;
}


static bool __dispatch__SEMICOLON(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__SEMICOLON()\n");
    return true;
}

static bool __dispatch__COMMA(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__COMMA()\n");
    return true;
}


static bool __dispatch__LPAREN(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__LPAREN()\n");
    return true;
}

static bool __dispatch__RPAREN(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__RPAREN()\n");
    return true;
}

static bool __dispatch__LCURLY(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__LCURLY()\n");
    return true;
}

static bool __dispatch__RCURLY(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__RCURLY()\n");
    return true;
}


static bool __dispatch__IDENTIFIER(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__IDENTIFIER()\n");
    return true;
}


static bool __dispatch__EOL(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__EOL()\n");
    return true;
}


static bool __dispatch__TIME_SIGN(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    TimeEvent *timeEvent = value;
    parseExpression(expression->left, context, &timeEvent->numerator, error);
    parseExpression(expression->left->right, context, &timeEvent->denominator, error);
    return true;
}

static bool __dispatch__SOUND_SELECT(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    SoundSelectEvent *event = NATypeNew(SoundSelectEvent, context->tick);
    event->channel = context->channel;

    Expression *expr = expression->left;

    do {
        if (INTEGER_LIST == expr->tokenType) {
            int32_t integerList[3];
            parseExpression(expr, context, integerList, error);
            event->msb = integerList[0];
            event->lsb = integerList[1];
            event->programNo = integerList[2];
        }
        else {
            parseExpression(expr, context, event, error);
        }
    } while ((expr = expr->right));

    ContextAddEvent(context, event);
    NARelease(event);

    return true;
}

static bool __dispatch__INTEGER_LIST(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    int32_t *integer = value;

    Expression *expr = expression->left;
    do {
        *integer = expr->v.i;
    } while ((++integer, expr = expr->right));

    return true;
}

static bool __dispatch__GATETIME_CUTOFF(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    parseExpression(expression->left, context, &context->gatetime, error);
    context->gatetime *= -1;
    return true;
}

static bool __dispatch__NOTE_BLOCK(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    int32_t step = *((int32_t *)value);
    NoteBlockContext *nbContext = NATypeNew(NoteBlockContext, context->tick, step);

    Expression *expr = expression->left;

    do {
        parseExpression(expr, context, nbContext, error);
        nbContext->tick += step;
    } while ((expr = expr->right));

    context->tick = nbContext->tick;

    CFArrayAppendArray(context->events, nbContext->events, CFRangeMake(0, CFArrayGetCount(nbContext->events)));

    NARelease(nbContext);

    return true;
}

static bool __dispatch__NOTE_NO_LIST(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    Expression *expr = expression->left;

    do {
        parseExpression(expr, context, value, error);
    } while ((expr = expr->right));

    return true;
}

static bool __dispatch__PATTERN_DEFINE(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__PATTERN_DEFINE()\n");
    return true;
}

static bool __dispatch__PATTERN_BLOCK(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__PATTERN_BLOCK()\n");
    return true;
}

static bool __dispatch__PATTERN_EXPAND(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__PATTERN_EXPAND()\n");
    return true;
}

static bool __dispatch__PATTERN_EXTEND_BLOCK(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__PATTERN_EXTEND_BLOCK()\n");
    return true;
}


#define SET_FUNCTION(type) dispatchTable[IDX(type)] = __dispatch__##type;

static void __attribute__((constructor)) initializeTable()
{
    SET_FUNCTION(INTEGER);
    SET_FUNCTION(FLOAT);
    SET_FUNCTION(STRING);

    SET_FUNCTION(NOTE_NO);
    SET_FUNCTION(LOCATION);
    SET_FUNCTION(MB_LENGTH);

    SET_FUNCTION(RESOLUTION);
    SET_FUNCTION(TITLE);
    SET_FUNCTION(TIME);
    SET_FUNCTION(TEMPO);
    SET_FUNCTION(MARKER);
    SET_FUNCTION(CHANNEL);
    SET_FUNCTION(VELOCITY);
    SET_FUNCTION(GATETIME);
    SET_FUNCTION(NOTE);

    SET_FUNCTION(STEP);
    SET_FUNCTION(FROM);
    SET_FUNCTION(TO);
    SET_FUNCTION(REPLACE);
    SET_FUNCTION(MIX);
    SET_FUNCTION(OFFSET);
    SET_FUNCTION(LENGTH);

    SET_FUNCTION(REST);
    SET_FUNCTION(TIE);

    SET_FUNCTION(PLUS);
    SET_FUNCTION(MINUS);
    SET_FUNCTION(DIVISION);
    SET_FUNCTION(MULTIPLY);
    SET_FUNCTION(ASSIGN);

    SET_FUNCTION(SEMICOLON);
    SET_FUNCTION(COMMA);

    SET_FUNCTION(LPAREN);
    SET_FUNCTION(RPAREN);
    SET_FUNCTION(LCURLY);
    SET_FUNCTION(RCURLY);

    SET_FUNCTION(IDENTIFIER);

    SET_FUNCTION(EOL);

    SET_FUNCTION(TIME_SIGN);
    SET_FUNCTION(SOUND_SELECT);
    SET_FUNCTION(INTEGER_LIST);
    SET_FUNCTION(GATETIME_CUTOFF);
    SET_FUNCTION(NOTE_BLOCK);
    SET_FUNCTION(NOTE_NO_LIST);
    SET_FUNCTION(PATTERN_DEFINE);
    SET_FUNCTION(PATTERN_BLOCK);
    SET_FUNCTION(PATTERN_EXPAND);
    SET_FUNCTION(PATTERN_EXTEND_BLOCK);
}
