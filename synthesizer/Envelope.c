#include "Envelope.h"

void EnvelopeInit(Envelope *self, EnvelopeType type)
{
    self->type = type;
    self->value = 0.0;
    self->phase = EnvelopePhaseDelay;
    self->startPhaseTime = 0.0;
    self->releasedValue = 0.0;
}

void EnvelopeUpdateRuntimeParams(Envelope *self,
        int16_t delay, int16_t attack, int16_t hold, int16_t decay, int16_t sustain, int16_t release,
        int16_t keynumToHold, int16_t keynumToDecay, int8_t keyForSample)
{
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

    // 37 sustainVolEnv/ 29 sustainModEnv
    // Values less than zero are to be interpreted as zero;
    // values above 1000 are to be interpreted as 1000.
    self->sustain = Clip(sustain, 0.0, 1000.0);

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

void EnvelopeRelease(Envelope *self, double time)
{
    if (EnvelopePhaseRelease > self->phase) {
        self->phase = EnvelopePhaseRelease;
        self->startPhaseTime = time;
        self->releasedValue = self->value;
    }
}

void EnvelopeFinish(Envelope *self, double time)
{
    // This method is for exclusiveClass
    // 8.1.1 Kinds of Generator Enumerators
    // 57 exclusiveClass - any other sounding note with the same exclusive class value should be rapidly terminated.

    if (EnvelopePhaseFinish > self->phase) {
        EnvelopeRelease(self, time);

        // Excerpt from fluid_synth.  Approximately 891 msec
        self->release = Timecent2Sec(-200);
    }
}

static void EnvelopeUpdatePhase(Envelope *self, double time)
{
    switch (self->phase) {
    case EnvelopePhaseDelay:
    case EnvelopePhaseAttack:
    case EnvelopePhaseHold:
    case EnvelopePhaseDecay:
        if ((self->startPhaseTime + self->phaseValues[self->phase]) <= time) {
            ++self->phase;
            self->startPhaseTime = time;
        }
        break;
    case EnvelopePhaseSustain:
        // Whenever a key-off occurs, the envelope immediately enters a release phase
        break;
    case EnvelopePhaseRelease:
        // -100dB
        if (0.00001 >= self->value) {
            ++self->phase;
            self->value = 0.0;
        }
        break;
    case EnvelopePhaseFinish:
        break;
    }
}

void EnvelopeUpdate(Envelope *self, double time)
{
    double f;

    switch (self->phase) {
    case EnvelopePhaseDelay:
        self->value = 0.0;
        break;
    case EnvelopePhaseAttack:
        // 9.1.7 Envelope Generators
        // The envelope then rises in a convex curve to a value of one during the attack phase.
        self->value = ConvexPositiveUnipolar((time - self->startPhaseTime) / self->attack);
        break;
    case EnvelopePhaseHold:
        self->value = 1.0;
        break;
    case EnvelopePhaseDecay:
        // 9.1.7 Envelope Generators
        // When the hold phase ends, the envelope enters a decay phase during which its value decreases linearly to a sustain level.
        f = (time - self->startPhaseTime) / self->decay;

        switch (self->type) {
        case EnvelopeTypeModulation:
            // The degree of modulation is specified in cents for the full-scale attack peak.
            self->value = 1.0 * (1.0 - f) + f * (1.0 - 0.001 * self->sustain);
            break;
        case EnvelopeTypeVolume:
            // The volume envelope operates in dB,
            self->value = cBAttn2Value(f * self->sustain);
            break;
        }
        break;
    case EnvelopePhaseSustain:
        switch (self->type) {
        case EnvelopeTypeModulation:
            // 8.1.1 Kinds of Generator Enumerators
            // This is the decrease in level, expressed in 0.1% units,
            // to which the Modulation Envelope value ramps during the decay phase.
            // For the Modulation Envelope, the sustain level is properly expressed
            // in percent of full scale. 
            self->value = 1.0 - 0.001 * self->sustain;
            break;
        case EnvelopeTypeVolume:
            // 9.1.7 Envelope Generators
            // The volume envelope operates in dB, with the attack peak providing a full scale output,
            // appropriately scaled by the initial volume.
            self->value = cBAttn2Value(self->sustain);
            break;
        }
        break;
    case EnvelopePhaseRelease:
        // 9.1.7 Envelope Generators
        // Whenever a key-off occurs, the envelope immediately enters a release phase
        // during which the value linearly ramps from the current value to zero.
        // When zero is reached, the envelope value remains at zero.
        f = (time - self->startPhaseTime) / self->release;

        switch (self->type) {
        case EnvelopeTypeModulation:
            // The degree of modulation is specified in cents for the full-scale attack peak.
            self->value = MAX(0.0, (1.0 - f) * self->releasedValue);
            break;
        case EnvelopeTypeVolume:
            // The volume envelope operates in dB,
            // If the current level were full scale, the Volume Envelope Release Time would be the time spent in release phase until 100dB attenuation were reached.
            self->value = self->releasedValue * cBAttn2Value(f * 1000.0);
            break;
        }
        break;
    case EnvelopePhaseFinish:
        break;
    }

    EnvelopeUpdatePhase(self, time);
}

bool EnvelopeIsReleased(Envelope *self)
{
    return EnvelopePhaseRelease == self->phase;
}

bool EnvelopeIsFinished(Envelope *self)
{
    return EnvelopePhaseFinish == self->phase;
}

double EnvelopeValue(Envelope *self)
{
    return self->value;
}
