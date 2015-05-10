#pragma once

#include <stdint.h>

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
    struct {
        uint16_t *buffer;
        uint32_t lenght;
    } smpl;
    struct {
        uint8_t *buffer;
        uint32_t lenght;
    } sm24;
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
