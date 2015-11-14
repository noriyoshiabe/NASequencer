#pragma once

#include "Expression.h"
#include "ABCParser.h"

#include <stdbool.h>

extern void *ABCExprStatementList(ABCParser *parser, ParseLocation *location);
extern void *ABCExprEOL(ABCParser *parser, ParseLocation *location);
extern void *ABCExprNote(ABCParser *parser, ParseLocation *location, char *noteString);

extern bool ABCExprIsStatementList(Expression *self);
extern Expression *ABCExprStatementListMarge(Expression *self, Expression *statementList);
