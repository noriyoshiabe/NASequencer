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
    if (!self->children) {
        self->children = NAArrayCreate(4, NULL);
    }
    NAArrayAppend(self->children, child);

    child->parent = self;
    return self;
}

void ExpressionDestroy(Expression *self)
{
    if (self->children) {
        NAArrayTraverse(self->children, ExpressionDestroy);
        NAArrayDestroy(self->children);
    }

    self->vtbl->destroy(self);
}

void ExpressionDump(Expression *self, int indent)
{
    self->vtbl->dump(self, indent);

    if (self->children) {
        int count = NAArrayCount(self->children);
        void **values = NAArrayGetValues(self->children);
        for (int i = 0; i < count; ++i) {
            ExpressionDump(values[i], indent + 4);
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
