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

typedef struct _EnumerationType {
    unsigned Type:6;
    unsigned P:1;
    unsigned D:1;
    unsigned CC:1;
    unsigned Index:7;
} __attribute__((__packed__)) EnumerationType;

typedef struct _EnumerationType SFModulator;
typedef struct _EnumerationType SFGenerator;
typedef struct _EnumerationType SFTransform;

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
