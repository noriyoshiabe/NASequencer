#pragma once

#include "SoundFont.h"
#include "Preset.h"

typedef enum {
    CC_BankSelect_MSB = 0,
    CC_Modulation_MSB = 1,
    CC_BreathController_MSB = 2,

    CC_FootController_MSB = 4,
    CC_Portamento_MSB = 5,
    CC_DataEntry_MSB = 6,
    CC_Volume_MSB = 7,
    CC_Balance_MSB = 8,

    CC_Pan_MSB = 10,
    CC_Expression_MSB = 11,
    CC_EffectController1_MSB = 12,
    CC_EffectController2_MSB = 13,

    CC_GeneralPurposeController1_MSB = 16,
    CC_GeneralPurposeController2_MSB = 17,
    CC_GeneralPurposeController3_MSB = 18,
    CC_GeneralPurposeController4_MSB = 19,

    CC_BankSelect_LSB = 32,
    CC_Modulation_LSB = 33,
    CC_BreathController_LSB = 34,

    CC_FootController_LSB = 36,
    CC_Portamento_LSB = 37,
    CC_DataEntry_LSB = 38,
    CC_Volume_LSB = 39,
    CC_Balance_LSB = 40,

    CC_Pan_LSB = 42,
    CC_Expression_LSB = 43,
    CC_EffectController1_LSB = 44,
    CC_EffectController2_LSB = 45,

    CC_GeneralPurposeController1_LSB = 48,
    CC_GeneralPurposeController2_LSB = 49,
    CC_GeneralPurposeController3_LSB = 50,
    CC_GeneralPurposeController4_LSB = 51,

    CC_Sustain = 64,
    CC_Portamento = 65,
    CC_Soft = 66,
    CC_Sostenuto = 67,
    CC_LegatoFootSwitch = 68,
    CC_Hold2 = 69,

    CC_SoundController1 = 70,
    CC_SoundController2 = 71,
    CC_SoundController3 = 72,
    CC_SoundController4 = 73,
    CC_SoundController5 = 74,
    CC_SoundController6 = 75,
    CC_SoundController7 = 76,
    CC_SoundController8 = 77,
    CC_SoundController9 = 78,
    CC_SoundController10 = 79,
    CC_GeneralPurposeController5 = 80,
    CC_GeneralPurposeController6 = 81,
    CC_GeneralPurposeController7 = 82,
    CC_GeneralPurposeController8 = 83,
    CC_PortamentoControl = 84,

    CC_Effect1Depth = 91,
    CC_Effect2Depth = 92,
    CC_Effect3Depth = 93,
    CC_Effect4Depth = 94,
    CC_Effect5Depth = 95,
    CC_DataIncrement = 96,
    CC_DataDecrement = 97,
    CC_NRPN_LSB = 98,
    CC_NRPN_MSB = 99,
    CC_RPN_LSB = 100,
    CC_RPN_MSB = 101,

    CC_AllSoundOff = 120,
    CC_ResetAllControllers = 121,
    CC_LocalControll = 122,
    CC_AllNotesOff = 123,
    CC_OmniModeOff = 124,
    CC_OmniModeOn = 125,
    CC_MonoMode = 126,
    CC_PolyMode = 127,
} ControllChange;

typedef enum {
    RPN_PitchBendSensitivity = 0,
    RPN_MasterFineTune = 1,
    RPN_MasterCoarseTune = 2,
    RPN_TuningProgramChange = 3,
    RPN_TuningBankSelect = 4,
    RPN_ModurationDepthRange = 5,
} RPN_Number;

typedef struct _Channel {
    uint8_t number;
    Preset *preset;

    int16_t keyPressure[128];
    int16_t channelPressure;
    int16_t pitchBend;
    int16_t pitchBendSensitivity;
    int16_t masterFineTune;
    int16_t masterCoarseTune;

    bool sustain;

    uint8_t cc[128];

    int16_t nrpnValues[SFGeneratorType_endOper];
    int32_t nrpnSelection;
    struct {
        int8_t _100;
        int8_t _1000;
        int8_t _10000;
    } nrpnMultiplier;

    bool nrpnActive;
    bool rpnActive;
} Channel;

extern void ChannelInitialize(Channel *self, uint8_t number, Preset *preset);
extern void ChannelSetControlChange(Channel *self, uint8_t ccNumber, uint8_t value);
extern uint16_t ChannelGetBankNumber(Channel *self);
