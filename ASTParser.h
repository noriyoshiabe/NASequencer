#pragma once

#include "Expression.h"
#include "Sequence.h"

typedef enum {
    ASTPARSER_NOERROR,
    ASTPARSER_RESOLUTION_REDEFINED,
    ASTPARSER_TITLE_REDEFINED,
} ASTParserErrorKind;

typedef struct _ASTParserError {
    ASTParserErrorKind kind;
    Expression *expression;
    const char *filepath;
    const char *message;
} ASTParserError;

extern Sequence *ASTParserParseExpression(Expression *expression, const char *filepath, ASTParserError *error);
