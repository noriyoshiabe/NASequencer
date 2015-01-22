#pragma once

#include "Expression.h"

typedef enum {
    DSLPARSER_NOERROR,
    DSLPARSER_FILE_NOT_FOUND,
    DSLPARSER_INIT_ERROR,
    DSLPARSER_PARSE_ERROR,
} DSLParserErrorKind;

typedef struct _DSLParserError {
    DSLParserErrorKind kind;
    const char *filepath;
    Location location;
    const char *message;
} DSLParserError;

extern Expression *DSLParserParseFile(const char *filepath, DSLParserError *error);
extern void DSLParserDumpExpression(Expression *expression);
extern void DSLParserDeleteExpression(Expression *expression);
