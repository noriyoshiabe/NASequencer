#pragma once

#include "SoundFont.h"

typedef struct _Synthesizer Synthesizer;
extern Synthesizer *SynthesizerCreate(SoundFont *sf, double sampleRate);
extern void SynthesizerDestroy(Synthesizer *self);
