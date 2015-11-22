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

void *ExpressionCreateListExpr(ParseLocation *location, int size, const char *identifier)
{
    Expression *self = ExpressionCreate(location, size, identifier);
    self->isList = true;
    return self;
}

Expression *ExpressionAddChild(Expression *self, Expression *child)
{
    bool isIdenticalListType = self->isList && child->isList && self->identifier == child->identifier;

    if (!self->children) {
        if (isIdenticalListType && child->children) {
            ExpressionDestroy(self);
            return child;
        }

        self->children = NAArrayCreate(4, NULL);
    }

    if (isIdenticalListType && child->children) {
        NAIterator *iterator = NAArrayGetIterator(child->children);
        while (iterator->hasNext(iterator)) {
            Expression *childExpr = iterator->next(iterator);
            childExpr->parent = self;
            NAArrayAppend(self->children, childExpr);
        }

        NAArrayRemoveAll(child->children);
        ExpressionDestroy(child);
    }
    else {
        NAArrayAppend(self->children, child);
        child->parent = self;
    }

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
        NAIterator *iterator = NAArrayGetIterator(self->children);
        while (iterator->hasNext(iterator)) {
            ExpressionDump(iterator->next(iterator), indent + 4);
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
#if 0
    Expression *self = _self;
    printf("parsed [%s]\n", self->identifier);
#endif
    return true;
}

static ExpressionVtbl _defaultVtbl = {
    _ExpressionDestroy,
    _ExpressionParse,
    _ExpressionDump,
};
