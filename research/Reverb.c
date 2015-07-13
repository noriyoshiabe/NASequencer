#include "Reverb.h"
#include "IIRFilter.h"

#include <stdlib.h>
#include <math.h>

struct _Reverb {
    IIRCombFilter *combFilters[4];
    IIRAllPassFilter *allPassFilters[2];
    IIRLowPassFilter *lowPassFilter;
};

Reverb *ReverbCreate(double sampleRate, double reverbTime)
{
    Reverb *self = calloc(1, sizeof(Reverb));

    // Reduce high frequency noise with pre LPF
    self->lowPassFilter = IIRLowPassFilterCreate(sampleRate, 1000.0, 1.0);

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

    self->combFilters[0] = IIRCombFilterCreate(sampleRate, CombFilterDelayTimes[0], pow(0.001, CombFilterDelayTimes[0] / reverbTime));
    self->combFilters[1] = IIRCombFilterCreate(sampleRate, CombFilterDelayTimes[1], pow(0.001, CombFilterDelayTimes[1] / reverbTime));
    self->combFilters[2] = IIRCombFilterCreate(sampleRate, CombFilterDelayTimes[2], pow(0.001, CombFilterDelayTimes[2] / reverbTime));
    self->combFilters[3] = IIRCombFilterCreate(sampleRate, CombFilterDelayTimes[3], pow(0.001, CombFilterDelayTimes[3] / reverbTime));

    self->allPassFilters[0] = IIRAllPassFilterCreate(sampleRate, AllPassFilterDelayTimes[0], 0.7);
    self->allPassFilters[1] = IIRAllPassFilterCreate(sampleRate, AllPassFilterDelayTimes[1], 0.7);

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

    IIRLowPassFilterDestroy(self->lowPassFilter);

    free(self);
}

AudioSample ReverbComputeSample(Reverb *self, AudioSample input)
{
    AudioSample combResults[4];

    input = IIRLowPassFilterApply(self->lowPassFilter, input);

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
