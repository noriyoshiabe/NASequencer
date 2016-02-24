#include "Reverb.h"
#include "Filter.h"

#include <stdlib.h>
#include <math.h>

struct _Reverb {
    CombFilter *combFilters[4];
    AllPassFilter *allPassFilters[2];
};

Reverb *ReverbCreate(double sampleRate, double reverbTime)
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

    self->combFilters[0] = CombFilterCreate(sampleRate, CombFilterDelayTimes[0], pow(0.001, CombFilterDelayTimes[0] / reverbTime));
    self->combFilters[1] = CombFilterCreate(sampleRate, CombFilterDelayTimes[1], pow(0.001, CombFilterDelayTimes[1] / reverbTime));
    self->combFilters[2] = CombFilterCreate(sampleRate, CombFilterDelayTimes[2], pow(0.001, CombFilterDelayTimes[2] / reverbTime));
    self->combFilters[3] = CombFilterCreate(sampleRate, CombFilterDelayTimes[3], pow(0.001, CombFilterDelayTimes[3] / reverbTime));

    self->allPassFilters[0] = AllPassFilterCreate(sampleRate, AllPassFilterDelayTimes[0], 0.7);
    self->allPassFilters[1] = AllPassFilterCreate(sampleRate, AllPassFilterDelayTimes[1], 0.7);

    return self;
}

void ReverbDestroy(Reverb *self)
{
    CombFilterDestroy(self->combFilters[0]);
    CombFilterDestroy(self->combFilters[1]);
    CombFilterDestroy(self->combFilters[2]);
    CombFilterDestroy(self->combFilters[3]);

    AllPassFilterDestroy(self->allPassFilters[0]);
    AllPassFilterDestroy(self->allPassFilters[1]);

    free(self);
}

AudioSample ReverbComputeSample(Reverb *self, AudioSample input)
{
    AudioSample combResults[4];

    combResults[0] = CombFilterApply(self->combFilters[0], input);
    combResults[1] = CombFilterApply(self->combFilters[1], input);
    combResults[2] = CombFilterApply(self->combFilters[2], input);
    combResults[3] = CombFilterApply(self->combFilters[3], input);

    AudioSample ret = {
        .L = combResults[0].L + combResults[1].L + combResults[2].L + combResults[3].L,
        .R = combResults[0].R + combResults[1].R + combResults[2].R + combResults[3].R,
    };

    ret = AllPassFilterApply(self->allPassFilters[0], ret);
    ret = AllPassFilterApply(self->allPassFilters[1], ret);

    return ret;
}
