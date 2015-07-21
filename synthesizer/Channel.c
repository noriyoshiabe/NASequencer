#include "Channel.h"
#include "Define.h"

#include <string.h>

static void ChannelResetAllControllers(Channel *self);

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
        // Supported but do nothing here
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
    case CC_ResetAllControllers:
        ChannelResetAllControllers(self);
        break;
    }
}

static void ChannelResetAllControllers(Channel *self)
{
    // Follow Recommended Practice (RP-015) by MIDI Manufacturers Association
    // http://www.midi.org/techspecs/rp15.php

    for (int i = 0; i < 128; ++i) {
        switch (i) {
        // Set Expression (#11) to 127
        case CC_Expression_LSB:
        case CC_Expression_MSB:
        // Set Registered and Non-registered parameter number LSB and MSB (#98-#101) to null value (127)
        case CC_RPN_LSB:
        case CC_RPN_MSB:
        case CC_NRPN_LSB:
        case CC_NRPN_MSB:
            self->cc[i] = 127;
            break;
        // Do NOT reset Bank Select (#0/#32)
        case CC_BankSelect_MSB:
        case CC_BankSelect_LSB:
        // Do NOT reset Volume (#7)
        case CC_Volume_MSB:
        case CC_Volume_LSB:
        // Do NOT reset Pan (#10)
        case CC_Pan_MSB:
        case CC_Pan_LSB:
        // Do NOT reset Sound Controllers (#70-#79) 
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
        // Do NOT reset Effect Controllers (#91-#95)
        case CC_Effect1Depth:
        case CC_Effect2Depth:
        case CC_Effect3Depth:
        case CC_Effect4Depth:
        case CC_Effect5Depth:
        // Do NOT reset other channel mode messages (#120-#127)
        case CC_AllSoundOff:
        case CC_ResetAllControllers:
        case CC_LocalControll:
        case CC_AllNotesOff:
        case CC_OmniModeOff:
        case CC_OmniModeOn:
        case CC_MonoMode:
        case CC_PolyMode:
            break;
        default:
            // Set Modulation (#1) to 0
            // Set Pedals (#64, #65, #66, #67) to 0
            // Any other controllers that a device can respond to should be set to 0
            self->cc[i] = 0;
            break;
        }
    }

    // Do NOT reset Program Change
    // Do NOT reset registered or non-registered parameters.

    // Reset polyphonic pressure for all notes to 0.
    for (int i = 0; i < 128; ++i) {
        self->keyPressure[i] = 0;
    }
    // Reset channel pressure to 0 
    self->channelPressure = 0;
    // Set pitch bender to center (64/0)
    self->pitchBend = 8192;

    // Since RPN/NRPN NULL is set
    self->nrpnActive = false;
    self->rpnActive = false;
}

uint16_t ChannelGetBankNumber(Channel *self)
{
    return self->cc[CC_BankSelect_MSB] << 7 | self->cc[CC_BankSelect_LSB];
}

bool ChannelIsSustained(Channel *self)
{
    return 64 <= self->cc[CC_Sustain];
}
