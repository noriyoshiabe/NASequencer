#pragma once

typedef enum ValueType {
    VALUE_TYPE_NONE,
    VALUE_TYPE_INTEGER,
    VALUE_TYPE_FLOAT,
    VALUE_TYPE_STRING
} ValueType;

typedef struct _Expression {
    int tokenType;
    ValueType valueType;
 
    union {
        int i;
        float f;
        char *s;
    } v;

    struct _Expression *left;
    struct _Expression *right;
    struct _Expression *next;
    struct _Expression *last;
} Expression;
 
extern Expression *createIntegerValue(int tokenType, int value);
extern Expression *createFloatValue(int tokenType, float value);
extern Expression *createStringValue(int tokenType, char *value);
extern Expression *createExpression(int tokenType, Expression *left, Expression *right);
extern Expression *addNextExpression(Expression *expr, Expression *next);
 
extern void dumpExpression(Expression *expr);
extern void deleteExpression(Expression *expr);
