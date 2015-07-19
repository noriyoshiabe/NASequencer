#include "Modulator.h"
#include "Define.h"

enum {
    PositiveUnipolar = (0 << 1) | 0,
    NegativeUnipolar = (0 << 1) | 1,
    PositiveBipolar  = (1 << 1) | 0,
    NegativeBipolar  = (1 << 1) | 1,
};

#define Curve(polarity, direction) ((polarity << 1) | direction)

int16_t ModulatorGetValue(const Modulator *self, Channel *channel, Voice *voice)
{
    double result = 0.0;

    const SFModulator *srcOpers[2] = {&self->sfModSrcOper, &self->sfModAmtSrcOper};
    for (int i = 0; i < 2; ++i) {
        double value;
        const SFModulator *srcOper = srcOpers[i];

        if (srcOper->CC) {
            value = channel->cc[srcOper->Index];
        }
        else {
            switch (srcOper->Index) {
            // No controller is to be used.
            // The output of this controller module should be treated as if its value were set to ‘1’.
            // It should not be a means to turn off a modulator.
            case SFGeneralControllerPalette_NoController:
                value = 1.0;
                break;
            case SFGeneralControllerPalette_NoteOnVelocity:
                value = (double)voice->velocity / 128.0;
                break;
            case SFGeneralControllerPalette_NoteOnKeyNumber:
                value = (double)voice->key / 128.0;
                break;
            case SFGeneralControllerPalette_PolyPressure:
                value = (double)channel->keyPressure / 128.0;
                break;
            case SFGeneralControllerPalette_ChannelPressure:
                value = (double)channel->channelPressure / 128.0;
                break;
            case SFGeneralControllerPalette_PitchWheel:
                value = (double)channel->pitchBend / 8192.0;
                break;
            case SFGeneralControllerPalette_PitchWheelSensitivity:
                value = (double)channel->pitchBendSensitivity / 128.0;
                break;
            case SFGeneralControllerPalette_Link:
                // TODO or NOT SUPPORT on first release
                value = 0.0;
                break;
            default:
                value = 0.0;
                break;
            }
        }

        switch (srcOper->Type) {
        case SFSourceType_Linear:
            switch (Curve(srcOper->P, srcOper->D)) {
            case PositiveUnipolar:
                break;
            case NegativeUnipolar:
                value = 1.0 - value;
                break;
            case PositiveBipolar:
                value = value * 2.0 - 1.0;
                break;
            case NegativeBipolar:
                value = 1.0 - value * 2.0;
                break;
            }
            break;
        case SFSourceType_Concave:
            switch (Curve(srcOper->P, srcOper->D)) {
            case PositiveUnipolar:
                value = ConcavePositiveUnipolar(value);
                break;
            case NegativeUnipolar:
                value = ConcaveNegativeUnipolar(value);
                break;
            case PositiveBipolar:
                value = ConcavePositiveBipolar(value);
                break;
            case NegativeBipolar:
                value = ConcaveNegativeBipolar(value);
                break;
            }
            break;
        case SFSourceType_Convex:
            switch (Curve(srcOper->P, srcOper->D)) {
            case PositiveUnipolar:
                value = ConvexPositiveUnipolar(value);
                break;
            case NegativeUnipolar:
                value = ConvexNegativeUnipolar(value);
                break;
            case PositiveBipolar:
                value = ConvexPositiveBipolar(value);
                break;
            case NegativeBipolar:
                value = ConvexNegativeBipolar(value);
                break;
            }
            break;
        case SFSourceType_Switch:
            switch (Curve(srcOper->P, srcOper->D)) {
            case PositiveUnipolar:
                value = SwitchPositiveUnipolar(value);
                break;
            case NegativeUnipolar:
                value = SwitchNegativeUnipolar(value);
                break;
            case PositiveBipolar:
                value = SwitchPositiveBipolar(value);
                break;
            case NegativeBipolar:
                value = SwitchNegativeBipolar(value);
                break;
            }
            break;
        }

        result += value;
    }

    result *= (double)self->modAmount;
    
    if (SFTransformType_AbsoluteValue == self->sfModTransOper) {
        result = fabs(result);
    }

    return round(result);
}

bool ModulatorIsIdentical(const Modulator *self, const Modulator *to)
{
    return self->sfModDestOper == to->sfModDestOper
        && *((uint16_t *)&self->sfModSrcOper) == *((uint16_t *)&to->sfModSrcOper)
        && *((uint16_t *)&self->sfModAmtSrcOper) == *((uint16_t *)&to->sfModAmtSrcOper)
        && self->sfModTransOper == to->sfModTransOper;
}

void ModulatorAddOverwrite(const Modulator **modList, int *count, const Modulator *toAdd)
{
    for (int i = 0; i < *count; ++i) {
        if (ModulatorIsIdentical(modList[i], toAdd)) {
            modList[i] = toAdd;
            return;
        }
    }

    modList[(*count)++] = toAdd;
}

