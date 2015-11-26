#pragma once

typedef struct _NAMidiComposer NAMidiComposer;

extern NAMidiComposer *NAMidiComposerCreate(void *parser);
extern void NAMidiComposerDestroy(NAMidiComposer *self);
