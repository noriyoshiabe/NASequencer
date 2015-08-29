#pragma once

#include "Parser.h"

#include <stdbool.h>

typedef struct _SequenceBuilder SequenceBuilder;

extern SequenceBuilder *SequenceBuilderCreate();
extern void SequenceBuilderDestroy(SequenceBuilder *self);
extern bool SequenceBuilderBuild(SequenceBuilder *self, ParseResult *result);

extern StatementHandler SequenceBuilderStatementHandler;
