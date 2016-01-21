#pragma once

#include "Define.h"

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    EnvelopeTypeModulation,
    EnvelopeTypeVolume,
} EnvelopeType;

typedef enum {
    EnvelopePhaseDelay,
    EnvelopePhaseAttack,
    EnvelopePhaseHold,
    EnvelopePhaseDecay,
    EnvelopePhaseSustain,
    EnvelopePhaseRelease,

    EnvelopePhaseFinish,
} EnvelopePhase;

typedef struct _Envelope {
    EnvelopeType type;

    union {
        struct {
            double delay;
            double attack;
            double hold;
            double decay;
            double sustain;
            double release;
        };

        double phaseValues[EnvelopePhaseFinish];
    };

    double value;

    EnvelopePhase phase;
    double startPhaseTime;
    double releasedValue;
    bool pendingRelease;
} Envelope;

extern void EnvelopeInit(Envelope *self, EnvelopeType type);
extern void EnvelopeUpdateRuntimeParams(Envelope *self,
        int16_t delay, int16_t attack, int16_t hold, int16_t decay, int16_t sustain, int16_t release,
        int16_t keynumToHold, int16_t keynumToDecay, int8_t keyForSample);
extern void EnvelopeRelease(Envelope *self, double time);
extern void EnvelopeFinish(Envelope *self, double time);
extern void EnvelopeUpdate(Envelope *self, double time);
extern bool EnvelopeIsReleased(Envelope *self);
extern bool EnvelopeIsFinished(Envelope *self);
extern double EnvelopeValue(Envelope *self);
