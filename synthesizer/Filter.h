#pragma once

typedef struct _LowPassFilter {
    struct {
        double a1;
        double a2;
        double b1;
        double b02;
    } coef;

    struct {
        double input;
        double output;
    } history[2];

    double last_frequency_cent;
} LowPassFilter;

extern LowPassFilter *LowPassFilterCreate(double sampleRate, double frequency, double q);
extern void LowPassFilterDestroy(LowPassFilter *self);
extern void LowPassFilterCalcLPFCoefficient(LowPassFilter *self, double sampleRate, double frequency_cent, double q_cB);
extern double LowPassFilterApply(LowPassFilter *self, double input);

typedef struct _CombFilter CombFilter;

extern CombFilter *CombFilterCreate(double sampleRate, double delay, double g);
extern void CombFilterDestroy(CombFilter *self);
extern double CombFilterApply(CombFilter *self, double input);

typedef struct _AllPassFilter AllPassFilter;

extern AllPassFilter *AllPassFilterCreate(double sampleRate, double delay, double g);
extern void AllPassFilterDestroy(AllPassFilter *self);
extern double AllPassFilterApply(AllPassFilter *self, double input);
