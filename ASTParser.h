#pragma once

#include "Expression.h"
#include "ParseError.h"
#include "Sequence.h"

#include <stdbool.h>

extern bool ASTParserParseExpression(Expression *expression, CFStringRef filepath, Sequence **sequence, CFMutableDictionaryRef *patterns, ParseError **error);
