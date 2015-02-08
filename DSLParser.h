#pragma once

#include "Expression.h"
#include "ParseError.h"

extern Expression *DSLParserParseFile(const char *filepath, ParseError *error);
extern void DSLParserDumpExpression(Expression *expression);
extern void DSLParserDeleteExpression(Expression *expression);
