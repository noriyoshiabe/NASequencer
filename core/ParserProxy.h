#pragma once

#include "Parser.h"
#include "Sequence.h"
#include "NAArray.h"

#include <stdbool.h>

typedef struct _ParserProxy ParserProxy;

extern ParserProxy *ParserProxyCreate();
extern void ParserProxyDestroy(ParserProxy *self);
extern bool ParserProxyParseFile(ParserProxy *self, const char *filepath, Sequence **sequence, ParseError *error, NAArray *filepaths);
extern const char *ParserProxyErrorDetail(ParseError *error);
