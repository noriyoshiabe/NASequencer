#pragma once

#include "Node.h"

typedef struct _NAMidiSEMParser NAMidiSEMParser;

extern NAMidiSEMParser *NAMidiSEMParserCreate(void *parser);
extern void NAMidiSEMParserDestroy(NAMidiSEMParser *self);
extern void *NAMidiSEMParserBuildSequence(NAMidiSEMParser *self, Node *node);
