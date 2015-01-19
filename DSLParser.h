#pragma once

#include "Expression.h"
#include <stdbool.h>

typedef enum {
    DSLPARSER_NOERROR,
    DSLPARSER_FILE_NOT_FOUND,
    DSLPARSER_INIT_ERROR,
    DSLPARSER_PARSE_ERROR,
} DSLParserError;

typedef struct _DSLParser {
    Expression *expression;
    DSLParserError error;
} DSLParser;

extern DSLParser *DSLParserCreate();
extern bool DSLParserParseFile(DSLParser *self, const char *filepath);
extern void DSLParserDumpExpression(DSLParser *self);
extern void DSLParserDestroy(DSLParser *self);
