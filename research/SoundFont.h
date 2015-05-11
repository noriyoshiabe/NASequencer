#pragma once

#include <stdint.h>

typedef struct _Version {
    uint16_t wMajor;
    uint16_t wMinor;
} Version;

typedef struct _PresetHeader {
    char achPresetName[20];
    uint16_t wPreset;
    uint16_t wBank;
    uint16_t wPresetBagNdx;
    uint32_t dwLibrary;
    uint32_t dwGenre;
    uint32_t dwMorphology;
} __attribute__((__packed__)) PresetHeader;

typedef struct _PresetBag {
    uint16_t wGenNdx;
    uint16_t wModNdx;
} PresetBag;

typedef struct _SFModulator {
    unsigned Type:6;
    unsigned P:1;
    unsigned D:1;
    unsigned CC:1;
    unsigned Index:7;
} __attribute__((__packed__)) SFModulator;

typedef enum {
    GeneralControllerPaletteNoController = 0,
    GeneralControllerPaletteNoteOnVelocity = 2,
    GeneralControllerPaletteNoteOnKeyNumber = 3,
    GeneralControllerPalettePolyPressure = 10,
    GeneralControllerPaletteChannelPressure = 13,
    GeneralControllerPalettePitchWheel = 14,
    GeneralControllerPalettePitchWheelSensitivity = 16,
    GeneralControllerPaletteLink = 127,
} GeneralControllerPalette;

typedef enum {
    SourceTypeLinear,
    SourceTypeConcave,
    SourceTypeConvex,
    SourceTypeSwitch,
} SourceType;

typedef uint16_t SFGenerator;
typedef enum {
    GeneratorType_startAddrsOffset,
    GeneratorType_endAddrsOffset,
    GeneratorType_startloopAddrsOffset,
    GeneratorType_endloopAddrsOffset,
    GeneratorType_startAddrsCoarseOffset,
    GeneratorType_modLfoToPitch,
    GeneratorType_vibLfoToPitch,
    GeneratorType_modEnvToPitch,
    GeneratorType_initialFilterFc,
    GeneratorType_initialFilterQ,
    GeneratorType_modLfoToFilterFc,
    GeneratorType_modEnvToFilterFc,
    GeneratorType_endAddrsCoarseOffset,
    GeneratorType_modLfoToVolume,
    GeneratorType_unused1,
    GeneratorType_chorusEffectsSend,
    GeneratorType_reverbEffectsSend,
    GeneratorType_pan,
    GeneratorType_unused2,
    GeneratorType_unused3,
    GeneratorType_unused4,
    GeneratorType_delayModLFO,
    GeneratorType_freqModLFO,
    GeneratorType_delayVibLFO,
    GeneratorType_freqVibLFO,
    GeneratorType_delayModEnv,
    GeneratorType_attackModEnv,
    GeneratorType_holdModEnv,
    GeneratorType_decayModEnv,
    GeneratorType_sustainModEnv,
    GeneratorType_releaseModEnv,
    GeneratorType_keynumToModEnvHold,
    GeneratorType_keynumToModEnvDecay,
    GeneratorType_delayVolEnv,
    GeneratorType_attackVolEnv,
    GeneratorType_holdVolEnv,
    GeneratorType_decayVolEnv,
    GeneratorType_sustainVolEnv,
    GeneratorType_releaseVolEnv,
    GeneratorType_keynumToVolEnvHold,
    GeneratorType_keynumToVolEnvDecay,
    GeneratorType_instrument,
    GeneratorType_reserved1,
    GeneratorType_keyRange,
    GeneratorType_velRange,
    GeneratorType_startloopAddrsCoarseOffset,
    GeneratorType_keynum,
    GeneratorType_velocity,
    GeneratorType_initialAttenuation,
    GeneratorType_reserved2,
    GeneratorType_endloopAddrsCoarseOffset,
    GeneratorType_coarseTune,
    GeneratorType_fineTune,
    GeneratorType_sampleID,
    GeneratorType_sampleModes,
    GeneratorType_reserved3,
    GeneratorType_scaleTuning,
    GeneratorType_exclusiveClass,
    GeneratorType_overridingRootKey,
    GeneratorType_unused5,
    GeneratorType_endOper,
} GeneratorType;

typedef uint16_t SFTransform;
typedef enum {
    TransformTypeLinear,
    TransformTypeAbsoluteValue,
} TransformType;

typedef struct _ModList {
    SFModulator sfModSrcOper;
    SFGenerator sfModDestOper;
    int16_t modAmount;
    SFModulator sfModAmtSrcOper;
    SFTransform sfModTransOper;
} ModList;

typedef struct _SoundFont {
    Version ifil;
    char *isng;
    char *INAM;
    char *irom;
    Version iver;
    char *ICRD;
    char *IENG;
    char *IPRD;
    char *ICOP;
    char *ICMT;
    char *ISFT;

    uint16_t *smpl;
    uint32_t smplLength;

    uint8_t *sm24;
    uint32_t sm24Length;

    PresetHeader *phdr;
    uint32_t phdrLength;
    PresetBag *pbag;
    uint32_t pbagLength;
    ModList *pmod;
    uint32_t pmodLength;
} SoundFont;

typedef enum {
    SoundFontErrorFileNotFound,
    SoundFontErrorUnsupportedVersion,
    SoundFontErrorInvalidFileFormat,
} SoundFontError;

extern SoundFont *SoundFontRead(const char *filepath, SoundFontError *error);
extern void SoundFontDestroy(SoundFont *self);
extern void SoundFontDump(SoundFont *self);

const char *SoundFontError2String(SoundFontError error)
{
#define CASE(error) case error: return &(#error[14]);
    switch (error) {
    CASE(SoundFontErrorFileNotFound);
    CASE(SoundFontErrorUnsupportedVersion);
    CASE(SoundFontErrorInvalidFileFormat);
    }
#undef CASE
    return "SoundFontErrorUnknown";
};
