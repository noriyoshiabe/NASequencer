#pragma once

#include "Expression.h"

typedef struct _ParseContext {
} ParseContext;

extern ParseContext *ASTParserParseExpression(Expression *expression);
