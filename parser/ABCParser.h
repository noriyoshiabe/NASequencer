#pragma once

#include "Parser.h"

#include <stdarg.h>

typedef enum {
    ABCExpressionTypeNote,

    ABCExpressionTypeCount,
} ABCExpressionType;

typedef struct _ABCParser ABCParser;

extern bool ABCParserProcess(ABCParser *self, int line, int column, ABCExpressionType type, va_list argList);
extern void ABCParserError(ABCParser *self, int line, int column, ParseErrorKind errorKind);

extern Parser *ABCParserCreate(ParseResult *result);

static inline const char *ABCExpressionType2String(ABCExpressionType type)
{
#define CASE(type) case type: return &(#type[17])
    switch (type) {
    CASE(ABCExpressionTypeNote);

    default:
       break;
    }

    return "Unknown expression type";
#undef CASE
}
