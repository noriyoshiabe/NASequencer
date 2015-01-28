#include "ASTParser.h"
#include "Parser.h"
#include <NACFHelper.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct _Context {
    Sequence *sequence;
    uint32_t tick;
    int32_t channel;
    int32_t gatetime;
    CFMutableArrayRef events;
    TimeTable *timeTable;
} Context;

static Context *createContext(Sequence *sequence)
{
    Context *context = calloc(1, sizeof(Context));
    context->sequence = NARetain(sequence);
    context->events = CFArrayCreateMutable(NULL, 0, NACFArrayCallBacks);
    context->timeTable = NATypeNew(TimeTable);
    return context;
}

static void ContextAddEvent(Context *context, void *event)
{
    CFArrayAppendValue(context->events, event);
}

static void destroyContext(Context *context)
{
    CFRelease(context->events);
    NARelease(context->sequence);
    NARelease(context->timeTable);
    free(context);
}

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
    Context *context = createContext(sequence);

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
    destroyContext(context);

    return sequence;
}

static bool __dispatch__INTEGER(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    *((int *)value) = expression->v.i;
    return true;
}

static bool __dispatch__FLOAT(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__FLOAT()\n");
    return true;
}

static bool __dispatch__STRING(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    *((char **)value) = strdup(expression->v.s);
    return true;
}


static bool __dispatch__NOTE_NO(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__NOTE_NO()\n");
    return true;
}

static bool __dispatch__LOCATION(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__LOCATION()\n");
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

    return parseExpression(expression->left, context, &context->sequence->resolution, error);
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
    TimeEvent *timeEvent = NATypeNew(TimeEvent, context->tick);

    Expression *expr = expression->left;

    do {
        parseExpression(expr, context, timeEvent, error);
    } while ((expr = expr->right));

    TimeTableAddTimeEvent(context->timeTable, timeEvent);
    ContextAddEvent(context, timeEvent);
    NARelease(timeEvent);

    return true;
}

static bool __dispatch__TEMPO(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__TEMPO()\n");
    return true;
}

static bool __dispatch__MARKER(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__MARKER()\n");
    return true;
}

static bool __dispatch__SOUND(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__SOUND()\n");
    return true;
}

static bool __dispatch__SELECT(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__SELECT()\n");
    return true;
}

static bool __dispatch__CHANNEL(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__CHANNEL()\n");
    return true;
}

static bool __dispatch__VELOCITY(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__VELOCITY()\n");
    return true;
}

static bool __dispatch__GATETIME(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__GATETIME()\n");
    return true;
}

static bool __dispatch__CUTOFF(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__CUTOFF()\n");
    return true;
}

static bool __dispatch__NOTE(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__NOTE()\n");
    return true;
}


static bool __dispatch__STEP(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__STEP()\n");
    return true;
}

static bool __dispatch__FROM(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    return parseExpression(expression->left, context, &((MidiEvent *)value)->tick, error);
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
    printf("called __dispatch__REST()\n");
    return true;
}

static bool __dispatch__TIE(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__TIE()\n");
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
    printf("called __dispatch__SOUND_SELECT()\n");
    return true;
}

static bool __dispatch__INTEGER_LIST(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__INTEGER_LIST()\n");
    return true;
}

static bool __dispatch__GATETIME_CUTOFF(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__GATETIME_CUTOFF()\n");
    return true;
}

static bool __dispatch__NOTE_BLOCK(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__NOTE_BLOCK()\n");
    return true;
}

static bool __dispatch__NOTE_NO_LIST(Expression *expression, Context *context, void *value, ASTParserError *error)
{
    printf("called __dispatch__NOTE_NO_LIST()\n");
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
    SET_FUNCTION(SOUND);
    SET_FUNCTION(SELECT);
    SET_FUNCTION(CHANNEL);
    SET_FUNCTION(VELOCITY);
    SET_FUNCTION(GATETIME);
    SET_FUNCTION(CUTOFF);
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
