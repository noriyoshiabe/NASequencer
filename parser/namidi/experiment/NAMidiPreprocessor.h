#pragma once

typedef struct _NAMidiPreprocessor NAMidiPreprocessor;

extern NAMidiPreprocessor *NAMidiPreprocessorCreate(void *parser);
extern void NAMidiPreprocessorDestroy(NAMidiPreprocessor *self);
