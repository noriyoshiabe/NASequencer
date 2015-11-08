#pragma once

#include "NAArray.h"
#include <stdbool.h>

typedef struct _ExpressionVtbl {
    void (*destroy)(void *self);
    bool (*parse)(void *self, void *visitor, void *context);
    void (*dump)(void *self, int indent);
} ExpressionVtbl;

typedef struct _ExpressionLocation {
    const char *filepath;
    int line;
    int column;
} ExpressionLocation;

typedef struct _Expression {
    ExpressionVtbl vtbl;
    const char *identifier;
    ExpressionLocation location;
    struct _Expression *parent;
    NAArray *children;
} Expression;
 
extern void *ExpressionCreate(const char *filepath, void *yylloc, int size, const char *identifier);
extern Expression *ExpressionAddChild(Expression *self, Expression *child);
extern void ExpressionDestroy(Expression *self);
extern void ExpressionDump(Expression *self, int indent);
extern bool ExpressionParse(Expression *self, void *visitor, void *context);
