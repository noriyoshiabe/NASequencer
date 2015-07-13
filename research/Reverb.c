#include "Reverb.h"

#include <stdlib.h>
#include <math.h>

typedef struct _IIRCombFilter {
    AudioSample *history;
    int historyLength;
    double g;
    int index;
} IIRCombFilter;

static IIRCombFilter *IIRCombFilterCreate(double delay, double g, double sampleRate)
{
    IIRCombFilter *self = calloc(1, sizeof(IIRCombFilter));

    int delayInSample = round(delay * sampleRate);
    self->history = calloc(delayInSample, sizeof(AudioSample));
    self->historyLength = delayInSample;
    self->g = g;
    self->index = 0;

    return self;
}

static void IIRCombFilterDestroy(IIRCombFilter *self)
{
    free(self->history);
    free(self);
}

static AudioSample IIRCombFilterApply(IIRCombFilter *self, AudioSample input)
{
    AudioSample output = self->history[self->index];
    
    self->history[self->index].L = input.L + output.L * self->g;
    self->history[self->index].R = input.R + output.R * self->g;
    
    if (self->historyLength <= ++self->index) {
        self->index = 0;
    }

    return output;
}

typedef struct _IIRAllPassFilter {
    AudioSample *history;
    int historyLength;
    double g;
    int index;
} IIRAllPassFilter;

static IIRAllPassFilter *IIRAllPassFilterCreate(double delay, double g, double sampleRate)
{
    IIRAllPassFilter *self = calloc(1, sizeof(IIRAllPassFilter));

    int delayInSample = round(delay * sampleRate);
    self->history = calloc(delayInSample, sizeof(AudioSample));
    self->historyLength = delayInSample;
    self->g = g;
    self->index = 0;

    return self;
}

static void IIRAllPassFilterDestroy(IIRAllPassFilter *self)
{
    free(self->history);
    free(self);
}

static AudioSample IIRAllPassFilterApply(IIRAllPassFilter *self, AudioSample input)
{
    AudioSample output = self->history[self->index];
    
    self->history[self->index].L = input.L + output.L * self->g;
    self->history[self->index].R = input.R + output.R * self->g;

    output.L -= input.L * self->g;
    output.R -= input.R * self->g;
    
    if (self->historyLength <= ++self->index) {
        self->index = 0;
    }

    return output;
}

struct _Reverb {
    IIRCombFilter *combFilters[4];
    IIRAllPassFilter *allPassFilters[2];
};

Reverb *ReverbCreate(double reverbTime, double sampleRate)
{
    Reverb *self = calloc(1, sizeof(Reverb));

    // Parameters are from the Schroeder Reverberator simulating a medium-sized concert hall
    // Excerpt from https://www.nativesystems.inf.ethz.ch/pub/Main/WebHomeLecturesReconfigurableSystems/lec7.pdf

    const double CombFilterDelayTimes[] = {
        0.0297,
        0.0371,
        0.0411,
        0.0437,
    };

    const double AllPassFilterDelayTimes[] = {
        0.0050,
        0.0017,
    };

    self->combFilters[0] = IIRCombFilterCreate(CombFilterDelayTimes[0], pow(0.001, CombFilterDelayTimes[0] / reverbTime), sampleRate);
    self->combFilters[1] = IIRCombFilterCreate(CombFilterDelayTimes[1], pow(0.001, CombFilterDelayTimes[1] / reverbTime), sampleRate);
    self->combFilters[2] = IIRCombFilterCreate(CombFilterDelayTimes[2], pow(0.001, CombFilterDelayTimes[2] / reverbTime), sampleRate);
    self->combFilters[3] = IIRCombFilterCreate(CombFilterDelayTimes[3], pow(0.001, CombFilterDelayTimes[3] / reverbTime), sampleRate);

    self->allPassFilters[0] = IIRAllPassFilterCreate(AllPassFilterDelayTimes[0], 0.7, sampleRate);
    self->allPassFilters[1] = IIRAllPassFilterCreate(AllPassFilterDelayTimes[1], 0.7, sampleRate);

    return self;
}

void ReverbDestroy(Reverb *self)
{
    IIRCombFilterDestroy(self->combFilters[0]);
    IIRCombFilterDestroy(self->combFilters[1]);
    IIRCombFilterDestroy(self->combFilters[2]);
    IIRCombFilterDestroy(self->combFilters[3]);

    IIRAllPassFilterDestroy(self->allPassFilters[0]);
    IIRAllPassFilterDestroy(self->allPassFilters[1]);

    free(self);
}

AudioSample ReverbComputeSample(Reverb *self, AudioSample input)
{
    AudioSample combResults[4];

    combResults[0] = IIRCombFilterApply(self->combFilters[0], input);
    combResults[1] = IIRCombFilterApply(self->combFilters[1], input);
    combResults[2] = IIRCombFilterApply(self->combFilters[2], input);
    combResults[3] = IIRCombFilterApply(self->combFilters[3], input);

    AudioSample ret = {
        .L = combResults[0].L + combResults[1].L + combResults[2].L + combResults[3].L,
        .R = combResults[0].R + combResults[1].R + combResults[2].R + combResults[3].R,
    };

    ret = IIRAllPassFilterApply(self->allPassFilters[0], ret);
    ret = IIRAllPassFilterApply(self->allPassFilters[1], ret);

    return ret;
}
