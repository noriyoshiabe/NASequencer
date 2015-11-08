#pragma once

#include "Parser.h"
#include "NAArray.h"
#include <stdbool.h>

typedef struct _ExpressionVtbl {
    void (*destroy)(void *self);
    bool (*parse)(void *self, void *visitor, void *context);
    void (*dump)(void *self, int indent);
} ExpressionVtbl;

typedef struct _Expression {
    ExpressionVtbl vtbl;
    const char *identifier;
    ParseLocation location;
    struct _Expression *parent;
    NAArray *children;
} Expression;
 
extern void *ExpressionCreate(ParseLocation *location, int size, const char *identifier);
extern Expression *ExpressionAddChild(Expression *self, Expression *child);
extern void ExpressionDestroy(Expression *self);
extern void ExpressionDump(Expression *self, int indent);
extern bool ExpressionParse(Expression *self, void *visitor, void *context);
