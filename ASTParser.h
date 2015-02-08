#pragma once

#include "Expression.h"
#include "ParseError.h"
#include "Sequence.h"

#include <stdbool.h>

extern bool ASTParserParseExpression(Expression *expression, const char *filepath, Sequence **sequence, CFMutableDictionaryRef *patterns, ParseError *error);
