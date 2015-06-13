#pragma once

#include <stdbool.h>

typedef struct _Synthesizer Synthesizer;
extern Synthesizer *SynthesizerCreate(const char *filepath);
extern void SynthesizerDestroy(Synthesizer *self);
extern void SynthesizerDump(Synthesizer *self);
