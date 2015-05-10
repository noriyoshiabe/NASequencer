#pragma once

#include <stdint.h>

typedef struct _SoundFont {
    struct {
        uint16_t major;
        uint16_t minor;
    } version;
} SoundFont;

typedef enum {
    SoundFontErrorFileNotFound,
    SoundFontErrorInvalidFileFormat,
} SoundFontError;

extern SoundFont *SoundFontRead(const char *filepath, SoundFontError *error);
extern void SoundFontDestroy(SoundFont *self);
extern void SoundFontDump(SoundFont *self);

const char *SoundFontError2String(SoundFontError error)
{
#define CASE(error) case error: return #error;
    switch (error) {
    CASE(SoundFontErrorFileNotFound);
    CASE(SoundFontErrorInvalidFileFormat);
    }
#undef CASE
    return "SoundFontErrorUnknown";
};
