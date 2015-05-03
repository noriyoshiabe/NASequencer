#pragma once

#include "ExpressionType.h"

typedef enum ValueType {
    ValueTypeNone,
    ValueTypeInteger,
    ValueTypeFloat,
    ValueTypeString
} ValueType;

typedef struct _ParseLocation {
    const char *filepath;
    int firstLine;
    int firstColumn;
} ParseLocation;

typedef struct _Expression {
    ParseLocation location;

    ExpressionType type;
    ValueType valueType;
 
    union {
        int i;
        float f;
        char *s;
    } v;

    struct _Expression *parent;
    struct _Expression *left;
    struct _Expression *right;
    struct _Expression *rightLast;
} Expression;
 
extern Expression *ExpressionCreateIntegerValue(void *scanner, void *yylloc, ExpressionType type, int value);
extern Expression *ExpressionCreateFloatValue(void *scanner, void *yylloc, ExpressionType type, float value);
extern Expression *ExpressionCreateStringValue(void *scanner, void *yylloc, ExpressionType type, char *value);
extern Expression *ExpressionCreateTrimmedStringValue(void *scanner, void *yylloc, ExpressionType type, char *value);
extern Expression *ExpressionCreate(void *scanner, void *yylloc, ExpressionType type, Expression *left, Expression *right);
extern Expression *ExpressionAddLeft(Expression *expr, Expression *left);
extern Expression *ExpressionAddRight(Expression *expr, Expression *right);
 
extern void ExpressionDump(Expression *expr);
extern void ExpressionDestroy(Expression *expr);
