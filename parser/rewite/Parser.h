#pragma once

#include "SequenceBuilder.h"
#include "ParseInfo.h"

typedef struct _Parser Parser;

extern Parser *ParserCreate(SequenceBuilder *builder);
extern void ParserDestroy(Parser *self);
extern void *ParserParseFile(Parser *self, const char *filepath, ParseInfo **info);
