#pragma once

#include "Node.h"

typedef struct _NAMidiASTParser NAMidiASTParser;

extern NAMidiASTParser *NAMidiASTParserCreate(void *parser);
extern void NAMidiASTParserDestroy(NAMidiASTParser *self);
extern Node *NAMidiASTParserBuildSemantics(NAMidiASTParser *self, Node *node);
