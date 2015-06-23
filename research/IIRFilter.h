#pragma once

#include "AudioSample.h"

typedef struct _IIRFilter {
    double sampleRate;

    struct {
        float a0;
        float a1;
        float a2;
        float b0;
        float b1;
        float b2;
    } coef;

    struct {
        AudioSample input;
        AudioSample output;
    } history[2];
} IIRFilter;

extern void IIRFilterInitialize(IIRFilter *self, double sampleRate);
extern void IIRFilterCalcLPFCoefficient(IIRFilter *self, double frequency, double q);
extern AudioSample IIRFilterApply(IIRFilter *self, AudioSample input);
