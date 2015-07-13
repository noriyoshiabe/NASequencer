#pragma once

#include "AudioSample.h"

typedef struct _IIRLowPassFilter {
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
} IIRLowPassFilter;

extern IIRLowPassFilter *IIRLowPassFilterCreate(double sampleRate, double frequency, double q);
extern void IIRLowPassFilterDestroy(IIRLowPassFilter *self);
extern void IIRLowPassFilterCalcLPFCoefficient(IIRLowPassFilter *self, double sampleRate, double frequency_cent, double q_cB);
extern AudioSample IIRLowPassFilterApply(IIRLowPassFilter *self, AudioSample input);

typedef struct _IIRCombFilter IIRCombFilter;

extern IIRCombFilter *IIRCombFilterCreate(double sampleRate, double delay, double g);
extern void IIRCombFilterDestroy(IIRCombFilter *self);
extern AudioSample IIRCombFilterApply(IIRCombFilter *self, AudioSample input);

typedef struct _IIRAllPassFilter IIRAllPassFilter;

extern IIRAllPassFilter *IIRAllPassFilterCreate(double sampleRate, double delay, double g);
extern void IIRAllPassFilterDestroy(IIRAllPassFilter *self);
extern AudioSample IIRAllPassFilterApply(IIRAllPassFilter *self, AudioSample input);
