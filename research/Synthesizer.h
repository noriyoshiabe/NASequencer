#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "AudioSample.h"

typedef struct _Synthesizer Synthesizer;
extern Synthesizer *SynthesizerCreate(const char *filepath, double sampleRate);
extern void SynthesizerComputeAudioSample(Synthesizer *self, AudioSample *buffer, uint32_t count);
extern int SynthesizerVoicingCount(Synthesizer *self);
extern void SynthesizerDestroy(Synthesizer *self);
