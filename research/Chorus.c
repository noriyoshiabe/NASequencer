#include "Chorus.h"

#include <stdlib.h>
#include <math.h>

Chorus *ChorusCreate(double sampleRate)
{
    Chorus *self = calloc(1, sizeof(Chorus));
    self->sampleRate = sampleRate;
    self->history = calloc((int)sampleRate, sizeof(AudioSample));
    self->historyLength = (int)sampleRate;
    return self;
}

void ChorusDestroy(Chorus *self)
{
    for (int i = 0; i < self->delayCount; ++i) {
        free(self->delays[i]);
    }

    if (self->delays) {
        free(self->delays);
    }

    free(self->history);
    free(self);
}

void ChorusAddDelay(Chorus *self, double delay, double frequency, double depth, double L, double R)
{
    int index = self->delayCount;
    self->delays = realloc(self->delays, ++self->delayCount * sizeof(ChorusDelay *));
    self->delays[index] = calloc(1, sizeof(ChorusDelay));

    self->delays[index]->delayInSample = round(delay * self->sampleRate);
    self->delays[index]->depthInSample = round(depth * self->sampleRate);
    
    self->delays[index]->lfoCoef = 2.0 * M_PI * frequency / self->sampleRate;
    self->delays[index]->phase = 0.0;

    self->delays[index]->level.L = L;
    self->delays[index]->level.R = R;
}

extern AudioSample ChorusComputeSample(Chorus *self, AudioSample input)
{
    AudioSample ret = { .L = 0.0, .R = 0.0 };

    for (int i = 0; i < self->delayCount; ++i) {
        ChorusDelay *delay = self->delays[i];

        double lfoValue = sin(delay->phase);
        int index = self->historyPointer - delay->delayInSample + round(delay->depthInSample * lfoValue);

        if (0 > index) {
            index += self->historyLength;
        }
        else if (self->historyLength <= index) {
            index %= self->historyLength;
        }

        ret.L += self->history[index].L * delay->level.L;
        ret.L += self->history[index].R * delay->level.R;

        delay->phase += delay->lfoCoef;
    }

    ++self->historyPointer;
    if (self->historyPointer >= self->historyLength) {
        self->historyPointer = 0;
    }

    self->history[self->historyPointer] = input;

    return ret;
}
