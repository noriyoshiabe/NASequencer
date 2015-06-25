#pragma once

#include "AudioSample.h"
#include <stdint.h>

typedef struct _IIRFilter {
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
} IIRFilter;

extern void IIRFilterCalcLPFCoefficient(IIRFilter *self, double sampleRate, double frequency, int16_t q_cB);
extern AudioSample IIRFilterApply(IIRFilter *self, AudioSample input);
