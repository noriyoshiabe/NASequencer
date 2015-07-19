#pragma once

#include <stdint.h>

typedef struct _SFVersion {
    uint16_t wMajor;
    uint16_t wMinor;
} SFVersion;

typedef struct _SFPresetHeader {
    char achPresetName[20];
    uint16_t wPreset;
    uint16_t wBank;
    uint16_t wPresetBagNdx;
    uint32_t dwLibrary;
    uint32_t dwGenre;
    uint32_t dwMorphology;
} __attribute__((__packed__)) SFPresetHeader;

typedef struct _SFPresetBag {
    uint16_t wGenNdx;
    uint16_t wModNdx;
} SFPresetBag;

typedef struct _SFModulator {
    unsigned Index:7;
    unsigned CC:1;
    unsigned D:1;
    unsigned P:1;
    unsigned Type:6;
} __attribute__((__packed__)) SFModulator;

typedef enum {
    SFGeneralControllerPalette_NoController = 0,
    SFGeneralControllerPalette_NoteOnVelocity = 2,
    SFGeneralControllerPalette_NoteOnKeyNumber = 3,
    SFGeneralControllerPalette_PolyPressure = 10,
    SFGeneralControllerPalette_ChannelPressure = 13,
    SFGeneralControllerPalette_PitchWheel = 14,
    SFGeneralControllerPalette_PitchWheelSensitivity = 16,
    SFGeneralControllerPalette_Link = 127,
} SFGeneralControllerPalette;

typedef enum {
    SFSourceType_Linear,
    SFSourceType_Concave,
    SFSourceType_Convex,
    SFSourceType_Switch,
} SFSourceType;

typedef uint16_t SFGenerator;
typedef enum {
    SFGeneratorType_startAddrsOffset,
    SFGeneratorType_endAddrsOffset,
    SFGeneratorType_startloopAddrsOffset,
    SFGeneratorType_endloopAddrsOffset,
    SFGeneratorType_startAddrsCoarseOffset,
    SFGeneratorType_modLfoToPitch,
    SFGeneratorType_vibLfoToPitch,
    SFGeneratorType_modEnvToPitch,
    SFGeneratorType_initialFilterFc,
    SFGeneratorType_initialFilterQ,
    SFGeneratorType_modLfoToFilterFc,
    SFGeneratorType_modEnvToFilterFc,
    SFGeneratorType_endAddrsCoarseOffset,
    SFGeneratorType_modLfoToVolume,
    SFGeneratorType_unused1,
    SFGeneratorType_chorusEffectsSend,
    SFGeneratorType_reverbEffectsSend,
    SFGeneratorType_pan,
    SFGeneratorType_unused2,
    SFGeneratorType_unused3,
    SFGeneratorType_unused4,
    SFGeneratorType_delayModLFO,
    SFGeneratorType_freqModLFO,
    SFGeneratorType_delayVibLFO,
    SFGeneratorType_freqVibLFO,
    SFGeneratorType_delayModEnv,
    SFGeneratorType_attackModEnv,
    SFGeneratorType_holdModEnv,
    SFGeneratorType_decayModEnv,
    SFGeneratorType_sustainModEnv,
    SFGeneratorType_releaseModEnv,
    SFGeneratorType_keynumToModEnvHold,
    SFGeneratorType_keynumToModEnvDecay,
    SFGeneratorType_delayVolEnv,
    SFGeneratorType_attackVolEnv,
    SFGeneratorType_holdVolEnv,
    SFGeneratorType_decayVolEnv,
    SFGeneratorType_sustainVolEnv,
    SFGeneratorType_releaseVolEnv,
    SFGeneratorType_keynumToVolEnvHold,
    SFGeneratorType_keynumToVolEnvDecay,
    SFGeneratorType_instrument,
    SFGeneratorType_reserved1,
    SFGeneratorType_keyRange,
    SFGeneratorType_velRange,
    SFGeneratorType_startloopAddrsCoarseOffset,
    SFGeneratorType_keynum,
    SFGeneratorType_velocity,
    SFGeneratorType_initialAttenuation,
    SFGeneratorType_reserved2,
    SFGeneratorType_endloopAddrsCoarseOffset,
    SFGeneratorType_coarseTune,
    SFGeneratorType_fineTune,
    SFGeneratorType_sampleID,
    SFGeneratorType_sampleModes,
    SFGeneratorType_reserved3,
    SFGeneratorType_scaleTuning,
    SFGeneratorType_exclusiveClass,
    SFGeneratorType_overridingRootKey,
    SFGeneratorType_unused5,

    SFGeneratorType_initialPitch, // This is undefined on SF2.4 Generator

    SFGeneratorType_endOper,
} SFGeneratorType;

typedef enum {
    SFSampleModeType_NoLoop = 0,
    SFSampleModeType_Continuously = 1,
    SFSampleModeType_Unused = 2,
    SFSampleModeType_UntilRelease = 3,
} SFSampleMode;

typedef uint16_t SFTransform;
typedef enum {
    SFTransformType_Linear,
    SFTransformType_AbsoluteValue,
} SFTransformType;

typedef struct _SFModList {
    SFModulator sfModSrcOper;
    SFGenerator sfModDestOper;
    int16_t modAmount;
    SFModulator sfModAmtSrcOper;
    SFTransform sfModTransOper;
} SFModList;

typedef union _SFGenAmoutType {
    struct {
        uint8_t byLo;
        uint8_t byHi;
    } ranges;
    int16_t shAmount;
    uint16_t wAmount;
} __attribute__((__packed__)) SFGenAmoutType;

typedef struct _SFGenList {
    SFGenerator sfGenOper;
    SFGenAmoutType genAmount;
} __attribute__((__packed__)) SFGenList;

typedef struct _SFInst {
    char achInstName[20];
    uint16_t wInstBagNdx;
} SFInst;

typedef struct _SFInstBag {
    uint16_t wInstGenNdx;
    uint16_t wInstModNdx;
} SFInstBag;

typedef uint16_t SFSampleLink;
typedef enum {
    SFSampleLinkType_monoSample = 1,
    SFSampleLinkType_rightSample = 2,
    SFSampleLinkType_leftSample = 4,
    SFSampleLinkType_linkedSample = 8,
    SFSampleLinkType_RomMonoSample = 0x8001,
    SFSampleLinkType_RomRightSample = 0x8002,
    SFSampleLinkType_RomLeftSample = 0x8004,
    SFSampleLinkType_RomLinkedSample = 0x8008
} SFSampleLinkType;

typedef struct _SFSampleHeader {
    char achSampleName[20];
    uint32_t dwStart;
    uint32_t dwEnd;
    uint32_t dwStartloop;
    uint32_t dwEndloop;
    uint32_t dwSampleRate;
    uint8_t byOriginalPitch;
    char chPitchCorrection;
    uint16_t wSampleLink;
    SFSampleLink sfSampleType;
} __attribute__((__packed__)) SFSampleHeader;

typedef struct _SoundFont {
    SFVersion ifil;
    char *isng;
    char *INAM;
    char *irom;
    SFVersion iver;
    char *ICRD;
    char *IENG;
    char *IPRD;
    char *ICOP;
    char *ICMT;
    char *ISFT;

    int16_t *smpl;
    uint32_t smplLength;

    int8_t *sm24;
    uint32_t sm24Length;

    SFPresetHeader *phdr;
    uint32_t phdrLength;
    SFPresetBag *pbag;
    uint32_t pbagLength;
    SFModList *pmod;
    uint32_t pmodLength;
    SFGenList *pgen;
    uint32_t pgenLength;
    SFInst *inst;
    uint32_t instLength;
    SFInstBag *ibag;
    uint32_t ibagLength;
    SFModList *imod;
    uint32_t imodLength;
    SFGenList *igen;
    uint32_t igenLength;
    SFSampleHeader *shdr;
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

static inline const char *SoundFontError2String(SoundFontError error)
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

static inline const char *SFGenerator2String(SFGenerator generator)
{
#define CASE(generator) case generator: return &(#generator[16]);
    switch (generator) {
    CASE(SFGeneratorType_startAddrsOffset);
    CASE(SFGeneratorType_endAddrsOffset);
    CASE(SFGeneratorType_startloopAddrsOffset);
    CASE(SFGeneratorType_endloopAddrsOffset);
    CASE(SFGeneratorType_startAddrsCoarseOffset);
    CASE(SFGeneratorType_modLfoToPitch);
    CASE(SFGeneratorType_vibLfoToPitch);
    CASE(SFGeneratorType_modEnvToPitch);
    CASE(SFGeneratorType_initialFilterFc);
    CASE(SFGeneratorType_initialFilterQ);
    CASE(SFGeneratorType_modLfoToFilterFc);
    CASE(SFGeneratorType_modEnvToFilterFc);
    CASE(SFGeneratorType_endAddrsCoarseOffset);
    CASE(SFGeneratorType_modLfoToVolume);
    CASE(SFGeneratorType_unused1);
    CASE(SFGeneratorType_chorusEffectsSend);
    CASE(SFGeneratorType_reverbEffectsSend);
    CASE(SFGeneratorType_pan);
    CASE(SFGeneratorType_unused2);
    CASE(SFGeneratorType_unused3);
    CASE(SFGeneratorType_unused4);
    CASE(SFGeneratorType_delayModLFO);
    CASE(SFGeneratorType_freqModLFO);
    CASE(SFGeneratorType_delayVibLFO);
    CASE(SFGeneratorType_freqVibLFO);
    CASE(SFGeneratorType_delayModEnv);
    CASE(SFGeneratorType_attackModEnv);
    CASE(SFGeneratorType_holdModEnv);
    CASE(SFGeneratorType_decayModEnv);
    CASE(SFGeneratorType_sustainModEnv);
    CASE(SFGeneratorType_releaseModEnv);
    CASE(SFGeneratorType_keynumToModEnvHold);
    CASE(SFGeneratorType_keynumToModEnvDecay);
    CASE(SFGeneratorType_delayVolEnv);
    CASE(SFGeneratorType_attackVolEnv);
    CASE(SFGeneratorType_holdVolEnv);
    CASE(SFGeneratorType_decayVolEnv);
    CASE(SFGeneratorType_sustainVolEnv);
    CASE(SFGeneratorType_releaseVolEnv);
    CASE(SFGeneratorType_keynumToVolEnvHold);
    CASE(SFGeneratorType_keynumToVolEnvDecay);
    CASE(SFGeneratorType_instrument);
    CASE(SFGeneratorType_reserved1);
    CASE(SFGeneratorType_keyRange);
    CASE(SFGeneratorType_velRange);
    CASE(SFGeneratorType_startloopAddrsCoarseOffset);
    CASE(SFGeneratorType_keynum);
    CASE(SFGeneratorType_velocity);
    CASE(SFGeneratorType_initialAttenuation);
    CASE(SFGeneratorType_reserved2);
    CASE(SFGeneratorType_endloopAddrsCoarseOffset);
    CASE(SFGeneratorType_coarseTune);
    CASE(SFGeneratorType_fineTune);
    CASE(SFGeneratorType_sampleID);
    CASE(SFGeneratorType_sampleModes);
    CASE(SFGeneratorType_reserved3);
    CASE(SFGeneratorType_scaleTuning);
    CASE(SFGeneratorType_exclusiveClass);
    CASE(SFGeneratorType_overridingRootKey);
    CASE(SFGeneratorType_unused5);
    CASE(SFGeneratorType_endOper);
    }
#undef CASE
    return "SFGenerator_unknown";
}

static inline const char *SFGeneralControllerPalette2String(SFGeneralControllerPalette palette)
{
#define CASE(palette) case palette: return &(#palette[27]);
    switch (palette) {
    CASE(SFGeneralControllerPalette_NoController);
    CASE(SFGeneralControllerPalette_NoteOnVelocity);
    CASE(SFGeneralControllerPalette_NoteOnKeyNumber);
    CASE(SFGeneralControllerPalette_PolyPressure);
    CASE(SFGeneralControllerPalette_ChannelPressure);
    CASE(SFGeneralControllerPalette_PitchWheel);
    CASE(SFGeneralControllerPalette_PitchWheelSensitivity);
    CASE(SFGeneralControllerPalette_Link);
    }
#undef CASE
    return "SFGeneralControllerPalette_unknown";
}

static inline const char *SFSourceType2String(SFSourceType type)
{
#define CASE(type) case type: return &(#type[13]);
    switch (type) {
    CASE(SFSourceType_Linear);
    CASE(SFSourceType_Concave);
    CASE(SFSourceType_Convex);
    CASE(SFSourceType_Switch);
    }
#undef CASE
    return "SFSourceType_unknown";
}

static inline const char *SFTransformType2String(SFTransformType type)
{
#define CASE(type) case type: return &(#type[16]);
    switch (type) {
    CASE(SFTransformType_Linear);
    CASE(SFTransformType_AbsoluteValue);
    }
#undef CASE
    return "SFTransformType_unknown";
}
