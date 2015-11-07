#pragma once

#include "Parser.h"

extern Parser *NAMidiParserCreate(SequenceBuilder *builder, ParserCallbacks *callbacks, void *receiver);
