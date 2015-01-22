#include "ASTParser.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

static void dispatchExpression(Expression *expression, ParseContext *context)
{
    // TODO 振り分け
    printf("dispatchExpression -- type=%d %d:%d\n", expression->tokenType, expression->location.firstLine,expression->location.firstColumn);
}

static bool parseExpression(Expression *expression, ParseContext *context)
{
    if (!expression) {
        return true;
    }

    Expression *expr = expression;

    do {
        dispatchExpression(expr, context);

        if (!parseExpression(expr->left, context)) {
            return false;
        }
    } while ((expr = expr->right));

    return true;
}

static void destroyParseContext(ParseContext *context)
{
    free(context);
}

ParseContext *ASTParserParseExpression(Expression *expression)
{
    ParseContext *ret = calloc(1, sizeof(ParseContext));

    if (!parseExpression(expression, ret)) {
        destroyParseContext(ret);
        ret = NULL;
    }

    return ret;
}
