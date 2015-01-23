#pragma once

#include "Expression.h"

typedef enum {
    ASTPARSER_NOERROR,
} ASTParserErrorKind;

typedef struct _ASTParserError {
    ASTParserErrorKind kind;
    Expression *expression;
    const char *filepath;
    const char *message;
} ASTParserError;

typedef struct _ParseContext {
} ParseContext;

extern ParseContext *ASTParserParseExpression(Expression *expression, const char *filepath, ASTParserError *error);
