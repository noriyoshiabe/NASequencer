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
    unsigned Type:6;
    unsigned P:1;
    unsigned D:1;
    unsigned CC:1;
    unsigned Index:7;
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
    SFGeneratorType_endOper,
} SFGeneratorType;

typedef enum {
    SFSampleModeType_NoLoop = 0,
    SFSampleModeType_Continuously = 1,
    SFSampleModeType_Unused = 2,
    SFSampleModeType_KeyDepression = 3,
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

typedef SFGenList SFInstGenList;

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

    uint16_t *smpl;
    uint32_t smplLength;

    uint8_t *sm24;
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
    SFInstGenList *igen;
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

inline const char *SoundFontError2String(SoundFontError error)
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
