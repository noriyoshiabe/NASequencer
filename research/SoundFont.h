#pragma once

#include <stdint.h>

typedef struct _PresetHeader PresetHeader;

typedef struct _SoundFont {
    struct {
        uint16_t wMajor;
        uint16_t wMinor;
    } ifil;
    char *isng;
    char *INAM;
    char *irom;
    struct {
        uint16_t wMajor;
        uint16_t wMinor;
    } iver;
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
} SoundFont;

struct _PresetHeader {
    char achPresetName[20];
    uint16_t wPreset;
    uint16_t wBank;
    uint16_t wPresetBagNdx;
    uint32_t dwLibrary;
    uint32_t dwGenre;
    uint32_t dwMorphology;
};

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
