#pragma once

typedef struct _NAMidiASTParser NAMidiASTParser;

extern NAMidiASTParser *NAMidiASTParserCreate();
extern void NAMidiASTParserDestroy(NAMidiASTParser *self);
