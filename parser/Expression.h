#pragma once

#include "NAArray.h"

typedef struct _ExpressionVtbl {
    void (*destroy)(void *self);
    void (*dump)(void *self, int indent);
    void (*parse)(void *self, void *context);
} ExpressionVtbl;

typedef struct _Expression {
    ExpressionVtbl *vtbl;

    struct {
        const char *filepath;
        int line;
        int column;
    } location;

    struct _Expression *parent;
    NAArray *children;

    const char *debug;
} Expression;
 
extern Expression *ExpressionCreate(const char *filepath, void *yylloc, int size, const char *debug);
extern Expression *ExpressionAddChild(Expression *self, Expression *child);
extern void ExpressionDestroy(Expression *self);
extern void ExpressionDump(Expression *self, int indent);
extern void ExpressionPrintLocation(void *self, int indent);
