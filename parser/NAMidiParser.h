#pragma once

#include "Parser.h"

extern Parser *NAMidiParserCreate(SequenceBuilder *builder, ParserCallbacks *callbacks, void *receiver);

#include "Expression.h"

typedef struct _NAMidiParser NAMidiParser;
extern bool NAMidiParserReadIncludeFile(NAMidiParser *self, const char *filepath, int line, int column, const char *includeFile, Expression **expression);
