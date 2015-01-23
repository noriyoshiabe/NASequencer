#pragma once

#include "Expression.h"
#include "Sequence.h"

typedef enum {
    ASTPARSER_NOERROR,
} ASTParserErrorKind;

typedef struct _ASTParserError {
    ASTParserErrorKind kind;
    Expression *expression;
    const char *filepath;
    const char *message;
} ASTParserError;

extern Sequence *ASTParserParseExpression(Expression *expression, const char *filepath, ASTParserError *error);
