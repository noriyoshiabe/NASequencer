#pragma once

#include "Define.h"

typedef struct _LFO {
    double delay;
    double value;
    double increment;
} LFO;

extern void LFOInit(LFO *self);
extern void LFOUpdateRuntimeParams(LFO *self, int16_t delay, int16_t frequency, double sampleRate);
extern void LFOUpdate(LFO *self, double time);
extern double LFOValue(LFO *self);
