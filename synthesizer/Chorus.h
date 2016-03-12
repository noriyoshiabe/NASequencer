#pragma once

#include "AudioSample.h"

typedef struct _ChorusDelay {
    int delayInSample;
    int depthInSample;
    double lfoCoef;
    double phase;
    struct {
        double L;
        double R;
    } level;
} ChorusDelay;

typedef struct _Chorus {
    double *history;
    int historyLength;
    int historyPointer;
    ChorusDelay **delays;
    int delayCount;
    double sampleRate;
} Chorus;

extern Chorus *ChorusCreate(double sampleRate);
extern void ChorusDestroy(Chorus *self);
extern void ChorusAddDelay(Chorus *self, double delay, double frequency, double depth, double L, double R);
extern AudioSample ChorusComputeSample(Chorus *self, double input);
