#pragma once

#include "Expression.h"
#include "ParseError.h"
#include "Sequence.h"

extern Sequence *ASTParserParseExpression(Expression *expression, const char *filepath, ParseError *error);
