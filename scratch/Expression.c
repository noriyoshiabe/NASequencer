#include "Expression.h"
#include "Parser.h"
#include "Lexer.h"
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline ParseLocation makeParseLocation(void *scanner, void *yylloc)
{
    ParseLocation ret;
    ret.filepath = ((ParseLocation *)yyget_extra(scanner))->filepath;
    ret.firstLine = ((YYLTYPE *)yylloc)->first_line;
    ret.firstColumn = ((YYLTYPE *)yylloc)->first_column;
    return ret;
}

Expression *ExpressionCreateIntegerValue(void *scanner, void *yylloc, ExpressionType type, int value)
{
    Expression *expr = (Expression *)calloc(1, sizeof(Expression));
    expr->location = makeParseLocation(scanner, yylloc);
    expr->type = type;
    expr->valueType = ValueTypeInteger;
    expr->v.i = value;
    return expr;
}

Expression *ExpressionCreateFloatValue(void *scanner, void *yylloc, ExpressionType type, int number, const char *decimal)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%d.%s", number, decimal);

    Expression *expr = (Expression *)calloc(1, sizeof(Expression));
    expr->location = makeParseLocation(scanner, yylloc);
    expr->type = type;
    expr->valueType = ValueTypeFloat;
    expr->v.f = atof(buf);
    return expr;
}

Expression *ExpressionCreateStringValue(void *scanner, void *yylloc, ExpressionType type, char *value)
{
    Expression *expr = (Expression *)calloc(1, sizeof(Expression));
    expr->location = makeParseLocation(scanner, yylloc);
    expr->type = type;
    expr->valueType = ValueTypeString;
    expr->v.s = value;
    return expr;
}

Expression *ExpressionCreateTrimmedStringValue(void *scanner, void *yylloc, ExpressionType type, char *value)
{
    Expression *expr = ExpressionCreateStringValue(scanner, yylloc, type, value);
    int len = strlen(value);
    char *str = malloc(len - 1);
    strncpy(str, value + 1, len - 2);
    expr->v.s = str;
    free(value);
    return expr;
}

Expression *ExpressionCreate(void *scanner, void *yylloc, ExpressionType type, Expression *left, Expression *right)
{
    Expression *expr = (Expression *)calloc(1, sizeof(Expression));
    expr->location = makeParseLocation(scanner, yylloc);
    expr->type = type;
    expr->left = left;
    expr->right = right;

    while (left) {
        left->parent = expr;
        left = left->right;
    }

    return expr;
}

Expression *ExpressionAddLeft(Expression *expr, Expression *left)
{
    expr->left = left;
    left->parent = expr;
    return expr;
}

Expression *ExpressionAddRight(Expression *expr, Expression *right)
{
    if (expr->rightLast) {
        expr->rightLast->right = right;
        expr->rightLast = right;
    } else {
        expr->right = expr->rightLast = right;
    }

    return expr;
}


static void dump(Expression *expr, int depth)
{
    if (!expr) {
        return;
    }

    static char indent[128];
    memset(indent, ' ', depth * 2);
    indent[depth * 2] = '\0';

    switch (expr->valueType) {
    case ValueTypeNone:
        printf("%s[%s]", indent, ExpressionType2String(expr->type));
        break;
    case ValueTypeInteger:
        printf("%s[%s] integer %d", indent, ExpressionType2String(expr->type), expr->v.i);
        break;
    case ValueTypeFloat:
        printf("%s[%s] float %f", indent, ExpressionType2String(expr->type), expr->v.f);
        break;
    case ValueTypeString:
        printf("%s[%s] string %s", indent, ExpressionType2String(expr->type), expr->v.s);
        break;
    }

    printf("    -- parent=[%s]", expr->parent ? ExpressionType2String(expr->parent->type) : "");
#if 0
    printf("    -- %s - %d:%d\n", expr->location.filepath, expr->location.firstLine, expr->location.firstColumn);
#else
    putc('\n', stdout);
#endif

    dump(expr->left, depth + 1);
    dump(expr->right, depth);
}

void ExpressionDump(Expression *expr)
{
    dump(expr, 0);
}
 
void ExpressionDestroy(Expression *expr)
{
    if (!expr) {
        return;
    }
 
    ExpressionDestroy(expr->left);
    ExpressionDestroy(expr->right);
 
    if (ValueTypeString == expr->valueType) {
        free(expr->v.s);
    }

    free(expr);
}
