#pragma once

#include "Expression.h"
#include "ParseError.h"

#include <stdbool.h>

extern bool DSLParserParseFile(const char *filepath, Expression **expression, ParseError *error);
