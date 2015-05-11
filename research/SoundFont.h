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

typedef union _AmoutType {
    struct {
        uint8_t byLo;
        uint8_t byHi;
    } ranges;
    int16_t shAmount;
    uint16_t wAmount;
} __attribute__((__packed__)) AmoutType;

typedef struct _GenList {
    SFGenerator sfGenOper;
    AmoutType genAmount;
} __attribute__((__packed__)) GenList;

typedef struct _Inst {
    char achInstName[20];
    uint16_t wInstBagNdx;
} Inst;

typedef struct _InstBag {
    uint16_t wInstGenNdx;
    uint16_t wInstModNdx;
} InstBag;

typedef enum {
    SampleLink_monoSample = 1,
    SampleLink_rightSample = 2,
    SampleLink_leftSample = 4,
    SampleLink_linkedSample = 8,
    SampleLink_RomMonoSample = 0x8001,
    SampleLink_RomRightSample = 0x8002,
    SampleLink_RomLeftSample = 0x8004,
    SampleLink_RomLinkedSample = 0x8008
} SampleLink;

typedef struct _SampleHeader {
    char achSampleName[20];
    uint32_t dwStart;
    uint32_t dwEnd;
    uint32_t dwStartloop;
    uint32_t dwEndloop;
    uint32_t dwSampleRate;
    uint8_t byOriginalPitch;
    char chPitchCorrection;
    uint16_t wSampleLink;
    uint16_t sfSampleType;
} __attribute__((__packed__)) SampleHeader;

typedef GenList InstGenList;

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
    GenList *pgen;
    uint32_t pgenLength;
    Inst *inst;
    uint32_t instLength;
    InstBag *ibag;
    uint32_t ibagLength;
    ModList *imod;
    uint32_t imodLength;
    InstGenList *igen;
    uint32_t igenLength;
    SampleHeader *shdr;
    uint32_t shdrLength;
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
