#include "Expression.h"
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _Location {
    int line;
    int column;
} Location;

static ExpressionVtbl _TEST;

Expression *ExpressionCreate(const char *filepath, void *yylloc, int size, const char *debug)
{
    Expression *self = calloc(1, size);
    self->vtbl = &_TEST;
    self->location.filepath = filepath;
    self->location.line = ((Location *)yylloc)->line;
    self->location.column = ((Location *)yylloc)->column;
    self->debug = debug;
    return self;
}

Expression *ExpressionAddChild(Expression *self, Expression *child)
{
    self->child = child;
    child->parent = self;
    return self;
}

Expression *ExpressionAddSibling(Expression *self, Expression *sibling)
{
    if (self->last) {
        self->last->next = sibling;
        self->last = sibling;
    } else {
        self->next = self->last = sibling;
    }

    return self;
}

void ExpressionDestroy(Expression *self)
{
    for (Expression *expr = self; expr; ) {
        Expression *toDestroy = expr;
        expr = expr->next;

        if (toDestroy->child) {
            ExpressionDestroy(toDestroy->child);
        }
        toDestroy->vtbl->destroy(toDestroy);
    }
}

void ExpressionDump(Expression *self, int indent)
{
    for (Expression *expr = self; expr; expr = expr->next) {
        expr->vtbl->dump(expr, indent);

        if (expr->child) {
            ExpressionDump(expr->child, indent + 4);
        }
    }
}

void ExpressionPrintLocation(void *_self, int indent)
{
    Expression *self = _self;
    printf("%s:%d:%d", self->location.filepath, self->location.line, self->location.column);
}

static void _ExpressionDump(void *_self, int indent)
{
    Expression *self = _self;
    printf("%*s", indent, "");
    printf("[%s]", self->debug);
    printf(" - ");
    ExpressionPrintLocation(self, indent);
    printf("\n");
}

static void _ExpressionDestroy(void *_self)
{
    Expression *self = _self;
    free(self);
}

static ExpressionVtbl _TEST = {
    _ExpressionDestroy,
    _ExpressionDump,
    NULL,
};
