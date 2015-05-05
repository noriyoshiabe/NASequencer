#include "Expression.h"
#include "YYContext.h"
#include "Parser.h"
#include "Lexer.h"
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline ParseLocation makeParseLocation(void *scanner, void *yylloc)
{
    ParseLocation ret;
    ret.filepath = ((YYContext *)yyget_extra(scanner))->location.filepath;
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

Expression *ExpressionCreateFloatValue(void *scanner, void *yylloc, ExpressionType type, float value)
{
    Expression *expr = (Expression *)calloc(1, sizeof(Expression));
    expr->location = makeParseLocation(scanner, yylloc);
    expr->type = type;
    expr->valueType = ValueTypeFloat;
    expr->v.f = value;
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

Expression *ExpressionCreate(void *scanner, void *yylloc, ExpressionType type, Expression *child)
{
    Expression *expr = (Expression *)calloc(1, sizeof(Expression));
    expr->location = makeParseLocation(scanner, yylloc);
    expr->type = type;
    expr->child = child;

    while (child) {
        child->parent = expr;
        child = child->next;
    }

    return expr;
}

Expression *ExpressionAddChild(Expression *expr, Expression *child)
{
    expr->child = child;
    child->parent = expr;
    return expr;
}

Expression *ExpressionAddSibling(Expression *expr, Expression *sibling)
{
    if (expr->last) {
        expr->last->next = sibling;
        expr->last = sibling;
    } else {
        expr->next = expr->last = sibling;
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

    dump(expr->child, depth + 1);
    dump(expr->next, depth);
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
 
    ExpressionDestroy(expr->child);
    ExpressionDestroy(expr->next);
 
    if (ValueTypeString == expr->valueType) {
        free(expr->v.s);
    }

    free(expr);
}
