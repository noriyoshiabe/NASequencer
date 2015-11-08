#include "Expression.h"
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ExpressionVtbl _defaultVtbl;

void *ExpressionCreate(ParseLocation *location, int size, const char *identifier)
{
    Expression *self = calloc(1, size);
    self->vtbl = _defaultVtbl;
    self->location = *location;
    self->identifier = identifier;
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

    self->vtbl.destroy(self);
}

void ExpressionDump(Expression *self, int indent)
{
    self->vtbl.dump(self, indent);
    printf(" - %s:%d:%d\n", self->location.filepath, self->location.line, self->location.column);

    if (self->children) {
        int count = NAArrayCount(self->children);
        void **values = NAArrayGetValues(self->children);
        for (int i = 0; i < count; ++i) {
            ExpressionDump(values[i], indent + 4);
        }
    }
}

bool ExpressionParse(Expression *self, void *visitor, void *context)
{
    return self->vtbl.parse(self, visitor, context);
}

static void _ExpressionDump(void *_self, int indent)
{
    Expression *self = _self;
    printf("%*s", indent, "");
    printf("[%s]", self->identifier);
}

static void _ExpressionDestroy(void *self)
{
    free(self);
}

static bool _ExpressionParse(void *_self, void *visitor, void *context)
{
    Expression *self = _self;
    printf("parsed [%s]\n", self->identifier);
    return true;
}

static ExpressionVtbl _defaultVtbl = {
    _ExpressionDestroy,
    _ExpressionParse,
    _ExpressionDump,
};
