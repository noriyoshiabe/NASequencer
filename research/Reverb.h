#pragma once

#include "AudioSample.h"

typedef struct _Reverb Reverb;

extern Reverb *ReverbCreate(double reverbTime, double sampleRate);
extern void ReverbDestroy(Reverb *self);
extern AudioSample ReverbComputeSample(Reverb *self, AudioSample input);
