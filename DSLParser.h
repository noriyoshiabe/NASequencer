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
    DSLParserError error;
    Location location;
    char *message;
} DSLParser;

extern DSLParser *DSLParserCreate();
extern bool DSLParserParseFile(DSLParser *self, const char *filepath, Expression **expression);
extern void DSLParserDestroy(DSLParser *self);
extern void DSLParserDumpExpression(Expression *expression);
extern void DSLParserDeleteExpression(Expression *expression);
