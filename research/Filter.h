#pragma once

#include "AudioSample.h"

typedef struct _LowPassFilter {
    struct {
        float a1;
        float a2;
        float b1;
        float b02;
    } coef;

    struct {
        AudioSample input;
        AudioSample output;
    } history[2];

    double last_frequency_cent;
} LowPassFilter;

extern LowPassFilter *LowPassFilterCreate(double sampleRate, double frequency, double q);
extern void LowPassFilterDestroy(LowPassFilter *self);
extern void LowPassFilterCalcLPFCoefficient(LowPassFilter *self, double sampleRate, double frequency_cent, double q_cB);
extern AudioSample LowPassFilterApply(LowPassFilter *self, AudioSample input);

typedef struct _CombFilter CombFilter;

extern CombFilter *CombFilterCreate(double sampleRate, double delay, double g);
extern void CombFilterDestroy(CombFilter *self);
extern AudioSample CombFilterApply(CombFilter *self, AudioSample input);

typedef struct _AllPassFilter AllPassFilter;

extern AllPassFilter *AllPassFilterCreate(double sampleRate, double delay, double g);
extern void AllPassFilterDestroy(AllPassFilter *self);
extern AudioSample AllPassFilterApply(AllPassFilter *self, AudioSample input);