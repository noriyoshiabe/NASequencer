#include "Channel.h"
#include "Define.h"

#include <string.h>

void ChannelInitialize(Channel *self, uint8_t number, Preset *preset)
{
    self->number = number;
    self->preset = preset;

    for (int i = 0; i < 128; ++i) {
        self->keyPressure[i] = 0;
    }

    self->channelPressure = 0;
    self->pitchBend = 8192;
    self->pitchBendSensitivity = 2; // 2 semi tones excerpt from fluid_synth
    self->masterFineTune = 0;
    self->masterCoarseTune = 0;
    self->sustain = false;

    for (int i = 0; i < 128; ++i) {
        switch (i) {
        case CC_Expression_LSB:
        case CC_Expression_MSB:
        case CC_RPN_LSB:
        case CC_RPN_MSB:
        case CC_NRPN_LSB:
        case CC_NRPN_MSB:
            self->cc[i] = 127;
            break;
        case CC_Volume_MSB:
            self->cc[i] = 100;
            break;
        case CC_Pan_MSB:
            self->cc[i] = 64;
            break;
        case CC_SoundController1:
        case CC_SoundController2:
        case CC_SoundController3:
        case CC_SoundController4:
        case CC_SoundController5:
        case CC_SoundController6:
        case CC_SoundController7:
        case CC_SoundController8:
        case CC_SoundController9:
        case CC_SoundController10:
            self->cc[i] = 64; // Excerpt from fluid_synth
            break;
        default:
            self->cc[i] = 0;
            break;
        }
    }
}

void ChannelSetControlChange(Channel *self, uint8_t ccNumber, uint8_t value)
{
    self->cc[ccNumber] = value;

    switch (ccNumber) {
    case CC_Sustain:
        self->sustain = 64 <= value;
        break;
    case CC_Soft:
    case CC_Sostenuto:
        // Not support
        break;
    case CC_RPN_MSB:
        self->nrpnActive = false;
        self->rpnActive = true;
        break;
    case CC_RPN_LSB:
        self->nrpnActive = false;
        self->rpnActive = true;

        // RPN NULL
        if (127 == self->cc[CC_RPN_MSB] && 127 == self->cc[CC_RPN_LSB]) {
            self->nrpnActive = false;
            self->rpnActive = false;
        }
        break;
    case CC_NRPN_MSB:
        self->nrpnActive = true;
        self->rpnActive = false;
        self->nrpnSelection = 0;
        memset(&self->nrpnMultiplier, 0, sizeof(self->nrpnMultiplier));
        break;
    case CC_NRPN_LSB:
        self->nrpnActive = true;
        self->rpnActive = false;

        // 9.6.2 The NRPN Select Values
        // The NRPN Select MSB message value is 120.
        // This message indicates that a NRPN Message that follows will be a SoundFont 2.01 NRPN message.
        if (120 == self->cc[CC_NRPN_MSB]) {
            switch (value) {
            case 100:
                ++self->nrpnMultiplier._100;
                break;
            case 101:
                ++self->nrpnMultiplier._1000;
                break;
            case 102:
                ++self->nrpnMultiplier._10000;
                break;
            default:
                if (100 > value) {
                    self->nrpnSelection = value + 
                        self->nrpnMultiplier._100 * 100 +
                        self->nrpnMultiplier._1000 * 1000 +
                        self->nrpnMultiplier._10000 * 1000;

                    memset(&self->nrpnMultiplier, 0, sizeof(self->nrpnMultiplier));
                }
                break;
            }
        }
        break;
    case CC_DataEntry_MSB:
        if (self->nrpnActive) {
            // 9.6.3 The Default Data Entry Ranges
            // The Data Entry value is “applied”to a generator at the time
            // the MSB message is sent in.
            
            if (120 == self->cc[CC_NRPN_MSB] && 100 > self->cc[CC_NRPN_LSB]
                    && self->nrpnSelection < SFGeneratorType_endOper) {
                int16_t data = (value << 7 | self->cc[CC_DataEntry_LSB]) - 8192;
                data = Clip(data, -8192, 8192);
                self->nrpnValues[self->nrpnSelection] = data * GeneratorNRPNScale(self->nrpnSelection);
            }
        }
        else if (self->rpnActive) {
            switch (self->cc[CC_RPN_LSB]) {
            case RPN_PitchBendSensitivity:
                self->pitchBendSensitivity = value;
                break;
            case RPN_MasterFineTune:
                {
                    // MSB unit is 1 cent (+/- 64 cent)
                    int16_t data = (value << 7 | self->cc[CC_DataEntry_LSB]) - 8192;
                    self->masterFineTune = round((double)data / 8192.0 * 64.0);
                }
                break;
            case RPN_MasterCoarseTune:
                // semitone
                self->masterCoarseTune = value - 64;
                break;
            case RPN_TuningProgramChange:
            case RPN_TuningBankSelect:
            case RPN_ModurationDepthRange:
                // Not support
                break;
            }
        }
        break;
    case CC_DataEntry_LSB:
        break;
    case CC_DataIncrement:
    case CC_DataDecrement:
        {
            int16_t data = (value << 7 | self->cc[CC_DataEntry_LSB]) - 8192;
            data += ccNumber == CC_DataIncrement ? 1 : -1;
            data = Clip(data, -8192, 8192);

            self->cc[CC_DataEntry_LSB] = data & 0x7F;
            self->cc[CC_DataEntry_MSB] = data >> 7;

            if (120 == self->cc[CC_NRPN_MSB] && 100 > self->cc[CC_NRPN_LSB]
                    && self->nrpnSelection < SFGeneratorType_endOper) {
                self->nrpnValues[self->nrpnSelection] = data * GeneratorNRPNScale(self->nrpnSelection);
            }
        }
        break;
    }
}

uint16_t ChannelGetBankNumber(Channel *self)
{
    return self->cc[CC_BankSelect_MSB] << 8 | self->cc[CC_BankSelect_LSB];
}
