#pragma once

typedef enum ValueType {
    VALUE_TYPE_NONE,
    VALUE_TYPE_INTEGER,
    VALUE_TYPE_FLOAT,
    VALUE_TYPE_STRING
} ValueType;

typedef struct _ParseLocation {
    int firstLine;
    int firstColumn;
    int lastLine;
    int lastColumn;
} ParseLocation;

typedef struct _Expression {
    ParseLocation location;

    int tokenType;
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
 
extern Expression *createIntegerValue(void *location, int tokenType, int value);
extern Expression *createFloatValue(void *location, int tokenType, float value);
extern Expression *createStringValue(void *location, int tokenType, char *value);
extern Expression *createExpression(void *location, int tokenType, Expression *left, Expression *right);
extern Expression *addLeftExpression(Expression *expr, Expression *left);
extern Expression *addRightExpression(Expression *expr, Expression *right);
 
extern const char *tokenType2String(int tokenType);
extern void dumpExpression(Expression *expr);
extern void deleteExpression(Expression *expr);