#pragma once

#include <stdint.h>

typedef enum {
    ADSREnvelopeTypeVolume,
    ADSREnvelopeTypeModulation,
} ADSREnvelopeType;

typedef enum {
    ADSREnvelopePhaseDelay,
    ADSREnvelopePhaseAttack,
    ADSREnvelopePhaseHold,
    ADSREnvelopePhaseDecay,
    ADSREnvelopePhaseSustain,
    ADSREnvelopePhaseRelease,

    ADSREnvelopePhaseFinish,
} ADSREnvelopePhase;

typedef struct _ADSREnvelope {
    double delay;
    double attack;
    double hold;
    double decay;
    double sustain;
    double release;

    double value;

    ADSREnvelopePhase phase;
    double startPhaseTime;
    double releasedValue;
} ADSREnvelope;

#define Timecent2Sec(tc) (pow(2.0, (double)tc / 1200.0))
#define cBAttn2Value(cb) (pow(10.0, ((double)-cb / 10.0) / 20.0))

static inline void ADSREnvelopeInit(ADSREnvelope *self, ADSREnvelopeType type,
        int16_t delay, int16_t attack, int16_t hold, int16_t decay, int16_t sustain, int16_t release,
        int16_t keynumToHold, int16_t keynumToDecay, int8_t keyForSample)
{
    self->value = 0.0;

    self->phase = ADSREnvelopePhaseDelay;
    self->startPhaseTime = 0.0;
    self->releasedValue = 0.0;

    // 8.1.1 Kinds of Generator Enumerators

    // 25 delayModEnv / 33 delayVolEnv
    // The most negative number (-32768) conventionally indicates no delay.
    if (-32768 >= delay) {
        self->delay = 0.0;
    }
    else {
        self->delay = Timecent2Sec(Clip(delay, -12000, 5000));
    }

    // 26 attackModEnv / 34 attackVolEnv
    // The most negative number (-32768) conventionally indicates instantaneous attack.
    if (-32768 >= attack) {
        self->attack = 0.0;
    }
    else {
        self->attack = Timecent2Sec(Clip(attack, -12000, 8000));
    }

    // 27 holdModEnv / 35 holdVolEnv
    // The most negative number (-32768) conventionally indicates no hold phase.
    if (-32768 >= hold) {
        self->hold = 0.0;
    }
    else {
        // 31 keynumToModEnvHold / 39 keynumToVolEnvHold
        // This is the degree, in timecents per KeyNumber units,
        // to which the hold time of the Volume Envelope is decreased by increasing MIDI key number.
        // The hold time at key number 60 is always unchanged.
        // The unit scaling is such that a value of 100 provides a hold time which tracks the keyboard;
        // that is, an upward octave causes the hold time to halve.
        double timecent = hold * pow(2.0, keynumToHold * (60.0 - keyForSample) / 1200.0);
        self->hold = Timecent2Sec(Clip(timecent, -12000, 5000));
    }

    switch (type) {
    case ADSREnvelopeTypeVolume:
        // 37 sustainVolEnv
        // This is the decrease in level, expressed in centibels,
        // to which the Volume Envelope value ramps during the decay phase.
        // For the Volume Envelope, the sustain level is best expressed
        // in centibels of attenuation from full scale.
        // A value of 0 indicates the sustain level is full level;
        // this implies a zero duration of decay phase regardless of decay time.
        // A positive value indicates a decay to the corresponding level.
        // Values less than zero are to be interpreted as zero;
        // conventionally 1000 indicates full attenuation.
        // For example, a sustain level which corresponds to
        // an absolute value 12dB below of peak would be 120.
        if (0 >= sustain) {
            self->sustain = 1.0;
        }
        else if (1000 <= sustain) {
            self->sustain = 0.0;
        }
        else {
            self->sustain = cBAttn2Value(sustain);
        }
        break;

    case ADSREnvelopeTypeModulation:
        // 29 sustainModEnv
        // to which the Modulation Envelope value ramps during the decay phase.
        // For the Modulation Envelope, the sustain level is properly expressed in percent of full scale.
        // Because the volume envelope sustain level is expressed as an attenuation from full scale,
        // the sustain level is analogously expressed as a decrease from full scale.
        // A value of 0 indicates the sustain level is full level;
        // this implies a zero duration of decay phase regardless of decay time.
        // A positive value indicates a decay to the corresponding level.
        // Values less than zero are to be interpreted as zero;
        // values above 1000 are to be interpreted as 1000.
        // For example, a sustain level which corresponds to an absolute value 40% of peak would be 600.
        if (0 >= sustain) {
            self->sustain = 1.0;
        }
        else if (1000 <= sustain) {
            self->sustain = 0.0;
        }
        else {
            self->sustain = 1.0 - 0.001 * Clip(sustain, 0, 1000);
        }
        break;
    }

    // 32 keynumToModEnvDecay / 40 keynumToVolEnvDecay
    // This is the degree, in timecents per KeyNumber units,
    // to which the hold time of the Volume Envelope is decreased by increasing MIDI key number.
    // The hold time at key number 60 is always unchanged.
    // The unit scaling is such that a value of 100 provides a hold time that tracks the keyboard;
    // that is, an upward octave causes the hold time to halve. 
    double timecent = decay * pow(2.0, keynumToDecay * (60.0 - keyForSample) / 1200.0);
    self->decay = Timecent2Sec(Clip(timecent, -12000, 8000));

    // 30 releaseModEnv / 38 releaseVolEnv
    self->release = Timecent2Sec(Clip(release, -12000, 8000));
}
