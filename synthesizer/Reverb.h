#pragma once

#include "AudioSample.h"

typedef struct _Reverb Reverb;

extern Reverb *ReverbCreate(double sampleRate, double reverbTime);
extern void ReverbDestroy(Reverb *self);
extern void ReverbComputeSample(Reverb *self, double input, AudioSample *output);
