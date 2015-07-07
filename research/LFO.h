#pragma once

#include "Define.h"

typedef struct _LFO {
    double delay;
    double value;

    // 9.1.6 Low Frequency Oscillators
    // An LFO is defined as having a delay period during which its value remains zero,
    // followed by a triangular waveshape ramping linearly to positive one,
    // then downward to negative 1, then upward again to positive one, etc.
    double increment;
} LFO;

static inline void LFOInit(LFO *self, int16_t delay, int16_t frequency, double sampleRate)
{
    // 8.1.1 Kinds of Generator Enumerators
    // 21 delayModLFO / 23 delayVibLFO
    // The most negative number (-32768) conventionally indicates no delay.
    if (-32768 >= delay) {
        self->delay = 0.0;
    }
    else {
        self->delay = Timecent2Sec(Clip(delay, -12000, 5000));
    }

    double samplePerCycle = sampleRate / AbsCent2Hz(frequency);
    self->increment = 1.0 / (samplePerCycle / 4.0);

    self->value = 0.0;
}

static inline void LFOUpdate(LFO *self, double time)
{
    if (time < self->delay) {
        return;
    }

    self->value += self->increment;
    if (1.0 < self->value) {
        self->value = 2.0 - self->value;
        self->increment = -self->increment;
    }
    else if (self->value < -1.0) {
        self->value = -2.0 - self->value;
        self->increment = -self->increment;
    }
}

static inline double LFOValue(LFO *self)
{
    return self->value;
}
