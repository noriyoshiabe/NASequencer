#include "Expression.h"
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Expression *createIntegerValue(int tokenType, int value)
{
    Expression *expr = (Expression *)calloc(1, sizeof(Expression));
    expr->tokenType = tokenType;
    expr->valueType = VALUE_TYPE_INTEGER;
    expr->v.i = value;
    return expr;
}

Expression *createFloatValue(int tokenType, float value)
{
    Expression *expr = (Expression *)calloc(1, sizeof(Expression));
    expr->tokenType = tokenType;
    expr->valueType = VALUE_TYPE_FLOAT;
    expr->v.f = value;
    return expr;
}

Expression *createStringValue(int tokenType, char *value)
{
    Expression *expr = (Expression *)calloc(1, sizeof(Expression));
    expr->tokenType = tokenType;
    expr->valueType = VALUE_TYPE_STRING;
    expr->v.s = value;
    return expr;
}

Expression *createExpression(int tokenType, Expression *left, Expression *right)
{
    Expression *expr = (Expression *)calloc(1, sizeof(Expression));
    expr->tokenType = tokenType;
    expr->left = left;
    expr->right = right;
    return expr;
}

Expression *addRightExpression(Expression *expr, Expression *right)
{
    if (expr->rightLast) {
        expr->rightLast->right = right;
        expr->rightLast = right;
    } else {
        expr->right = expr->rightLast = right;
    }

    return expr;
}


#include "Parser.h"

static const char *tokenType2String(int tokenType)
{
#define CASE(type) case type: return #type
    switch (tokenType) {
    CASE(INTEGER);
    CASE(FLOAT);
    CASE(STRING);
    CASE(NOTE_NO);
    CASE(LOCATION);
    CASE(MB_LENGTH);
    CASE(RESOLUTION);
    CASE(TITLE);
    CASE(TIME);
    CASE(TEMPO);
    CASE(MARKER);
    CASE(SOUND);
    CASE(SELECT);
    CASE(CHANNEL);
    CASE(VELOCITY);
    CASE(GATETIME);
    CASE(CUTOFF);
    CASE(NOTE);
    CASE(STEP);
    CASE(FROM);
    CASE(TO);
    CASE(REPLACE);
    CASE(MIX);
    CASE(OFFSET);
    CASE(LENGTH);
    CASE(REST);
    CASE(TIE);
    CASE(PLUS);
    CASE(MINUS);
    CASE(DIVISION);
    CASE(MULTIPLY);
    CASE(ASSIGN);
    CASE(SEMICOLON);
    CASE(COMMA);
    CASE(LPAREN);
    CASE(RPAREN);
    CASE(LCURLY);
    CASE(RCURLY);
    CASE(IDENTIFIER);
    CASE(EOL);
    CASE(TIME_SIGN);
    CASE(SOUND_SELECT);
    CASE(INTEGER_LIST);
    CASE(GATETIME_CUTOFF);
    CASE(NOTE_BLOCK);
    CASE(NOTE_NO_LIST);
    CASE(PATTERN_DEFINE);
    CASE(PATTERN_BLOCK);
    }
    return "Unknown token type";
#undef CASE
}

static char indent[128];

void dumpExpressionImpl(Expression *expr, int depth)
{
    if (!expr) {
        return;
    }

    memset(indent, ' ', depth * 2);
    indent[depth * 2] = '\0';

    switch (expr->valueType) {
    case VALUE_TYPE_NONE:
        printf("%s[%s]\n", indent, tokenType2String(expr->tokenType));
        break;
    case VALUE_TYPE_INTEGER:
        printf("%s[%s] integer %d\n", indent, tokenType2String(expr->tokenType), expr->v.i);
        break;
    case VALUE_TYPE_FLOAT:
        printf("%s[%s] float %f\n", indent, tokenType2String(expr->tokenType), expr->v.f);
        break;
    case VALUE_TYPE_STRING:
        printf("%s[%s] string %s\n", indent, tokenType2String(expr->tokenType), expr->v.s);
        break;
    }

    dumpExpressionImpl(expr->left, depth + 1);
    dumpExpressionImpl(expr->right, depth);
}

void dumpExpression(Expression *expr)
{
    dumpExpressionImpl(expr, 0);
}
 
void deleteExpression(Expression *expr)
{
    if (!expr)
        return;
 
    deleteExpression(expr->left);
    deleteExpression(expr->right);
 
    free(expr);
}
