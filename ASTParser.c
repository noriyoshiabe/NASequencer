#include "ASTParser.h"
#include "Parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct _ParseContext {
    Sequence *sequence;
    int patternNest;
} ParseContext;

#define TABLE_SIZE (TOKEN_END - TOKEN_BEGIN)
#define IDX(type) (type - TOKEN_BEGIN - 1)

static bool (*dispatchTable[TABLE_SIZE])(Expression *, ParseContext *, ASTParserError *) = {NULL};

static bool parseExpression(Expression *expression, ParseContext *context, ASTParserError *error)
{
    bool (*function)(Expression *, ParseContext *, ASTParserError *)= dispatchTable[IDX(expression->tokenType)];
    if (!function) {
        fprintf(stderr, "Dispatch function is not found. tokenType=%s\n", tokenType2String(expression->tokenType));
        abort();
    }
    return function(expression, context, error);
    
}

static void destroyParseContext(ParseContext *context)
{
    free(context);
}

Sequence *ASTParserParseExpression(Expression *expression, const char *filepath, ASTParserError *error)
{
    Sequence *ret = NULL;

    ParseContext *context = calloc(1, sizeof(ParseContext));
    context->sequence = NATypeNew(Sequence);

    error->filepath = filepath;

    Expression *expr = expression;

    do {
        if (!parseExpression(expr, context, error)) {
            NARelease(context->sequence);
            goto ERROR;
        }
    } while ((expr = expr->right));

ERROR:
    destroyParseContext(context);

    return ret;
}

static bool __dispatch__INTEGER(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__INTEGER()\n");
    return true;
}

static bool __dispatch__FLOAT(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__FLOAT()\n");
    return true;
}

static bool __dispatch__STRING(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__STRING()\n");
    return true;
}


static bool __dispatch__NOTE_NO(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__NOTE_NO()\n");
    return true;
}

static bool __dispatch__LOCATION(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__LOCATION()\n");
    return true;
}

static bool __dispatch__MB_LENGTH(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__MB_LENGTH()\n");
    return true;
}


static bool __dispatch__RESOLUTION(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__RESOLUTION()\n");
    return true;
}

static bool __dispatch__TITLE(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__TITLE()\n");
    return true;
}

static bool __dispatch__TIME(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__TIME()\n");
    return true;
}

static bool __dispatch__TEMPO(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__TEMPO()\n");
    return true;
}

static bool __dispatch__MARKER(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__MARKER()\n");
    return true;
}

static bool __dispatch__SOUND(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__SOUND()\n");
    return true;
}

static bool __dispatch__SELECT(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__SELECT()\n");
    return true;
}

static bool __dispatch__CHANNEL(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__CHANNEL()\n");
    return true;
}

static bool __dispatch__VELOCITY(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__VELOCITY()\n");
    return true;
}

static bool __dispatch__GATETIME(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__GATETIME()\n");
    return true;
}

static bool __dispatch__CUTOFF(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__CUTOFF()\n");
    return true;
}

static bool __dispatch__NOTE(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__NOTE()\n");
    return true;
}


static bool __dispatch__STEP(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__STEP()\n");
    return true;
}

static bool __dispatch__FROM(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__FROM()\n");
    return true;
}

static bool __dispatch__TO(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__TO()\n");
    return true;
}

static bool __dispatch__REPLACE(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__REPLACE()\n");
    return true;
}

static bool __dispatch__MIX(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__MIX()\n");
    return true;
}

static bool __dispatch__OFFSET(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__OFFSET()\n");
    return true;
}

static bool __dispatch__LENGTH(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__LENGTH()\n");
    return true;
}


static bool __dispatch__REST(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__REST()\n");
    return true;
}

static bool __dispatch__TIE(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__TIE()\n");
    return true;
}


static bool __dispatch__PLUS(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__PLUS()\n");
    return true;
}

static bool __dispatch__MINUS(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__MINUS()\n");
    return true;
}

static bool __dispatch__DIVISION(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__DIVISION()\n");
    return true;
}

static bool __dispatch__MULTIPLY(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__MULTIPLY()\n");
    return true;
}

static bool __dispatch__ASSIGN(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__ASSIGN()\n");
    return true;
}


static bool __dispatch__SEMICOLON(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__SEMICOLON()\n");
    return true;
}

static bool __dispatch__COMMA(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__COMMA()\n");
    return true;
}


static bool __dispatch__LPAREN(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__LPAREN()\n");
    return true;
}

static bool __dispatch__RPAREN(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__RPAREN()\n");
    return true;
}

static bool __dispatch__LCURLY(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__LCURLY()\n");
    return true;
}

static bool __dispatch__RCURLY(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__RCURLY()\n");
    return true;
}


static bool __dispatch__IDENTIFIER(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__IDENTIFIER()\n");
    return true;
}


static bool __dispatch__EOL(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__EOL()\n");
    return true;
}


static bool __dispatch__TIME_SIGN(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__TIME_SIGN()\n");
    return true;
}

static bool __dispatch__SOUND_SELECT(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__SOUND_SELECT()\n");
    return true;
}

static bool __dispatch__INTEGER_LIST(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__INTEGER_LIST()\n");
    return true;
}

static bool __dispatch__GATETIME_CUTOFF(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__GATETIME_CUTOFF()\n");
    return true;
}

static bool __dispatch__NOTE_BLOCK(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__NOTE_BLOCK()\n");
    return true;
}

static bool __dispatch__NOTE_NO_LIST(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__NOTE_NO_LIST()\n");
    return true;
}

static bool __dispatch__PATTERN_DEFINE(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__PATTERN_DEFINE()\n");
    return true;
}

static bool __dispatch__PATTERN_BLOCK(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__PATTERN_BLOCK()\n");
    return true;
}

static bool __dispatch__PATTERN_EXPAND(Expression *expression, ParseContext *context, ASTParserError *error)
{
    printf("called __dispatch__PATTERN_EXPAND()\n");
    return true;
}

static bool __dispatch__PATTERN_EXTEND_BLOCK(Expression *expression, ParseContext *context, ASTParserError *error)
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
