#pragma once

#include "SequenceBuilder.h"
#include "ParseResult.h"

typedef struct _Parser Parser;

extern Parser *ParserCreate(SequenceBuilder *builder);
extern void ParserDestroy(Parser *self);
extern ParseResult *ParserParseFile(Parser *self, const char *filepath);
