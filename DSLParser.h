#pragma once

#include "Expression.h"
#include "ParseError.h"

#include <stdbool.h>

extern bool DSLParserParseFile(CFStringRef filepath, Expression **expression, ParseError **error);
