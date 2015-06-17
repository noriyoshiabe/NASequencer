#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "AudioSample.h"

typedef struct _Synthesizer Synthesizer;
extern Synthesizer *SynthesizerCreate(const char *filepath);
extern void SynthesizerComputeAudioSample(Synthesizer *self, uint32_t sampleRate, AudioSample *buffer, uint32_t count);
extern int SynthesizerVoicingCount(Synthesizer *self);
extern void SynthesizerDestroy(Synthesizer *self);
