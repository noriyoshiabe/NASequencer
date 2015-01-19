#pragma once

typedef enum ValueType {
    VALUE_TYPE_NONE,
    VALUE_TYPE_INTEGER,
    VALUE_TYPE_FLOAT,
    VALUE_TYPE_STRING
} ValueType;

typedef struct _Location {
    int firstLine;
    int firstColumn;
    int lastLine;
    int lastColumn;
} Location;

typedef struct _Expression {
    Location location;

    int tokenType;
    ValueType valueType;
 
    union {
        int i;
        float f;
        char *s;
    } v;

    struct _Expression *left;
    struct _Expression *right;
    struct _Expression *rightLast;
} Expression;
 
extern Expression *createIntegerValue(void *location, int tokenType, int value);
extern Expression *createFloatValue(void *location, int tokenType, float value);
extern Expression *createStringValue(void *location, int tokenType, char *value);
extern Expression *createExpression(void *location, int tokenType, Expression *left, Expression *right);
extern Expression *addRightExpression(Expression *expr, Expression *right);
 
extern void dumpExpression(Expression *expr);
extern void deleteExpression(Expression *expr);
