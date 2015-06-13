#pragma once

#include "SoundFont.h"

typedef struct _Preset Preset;
typedef struct _Instrument Instrument;
typedef struct _Zone Zone;
typedef struct _Range Range;
typedef struct _SampleInfo SampleInfo;
typedef struct _Sample Sample;

struct _Preset {
    const char *name;
    uint16_t midiPresetNo;
    uint16_t bankNo;

    Zone *globalZone;
    Zone **zones;
    int zoneCount;
};

struct _Instrument {
    const char *name;

    Zone *globalZone;
    Zone **zones;
    int zoneCount;
};

struct _Range {
    uint8_t low;
    uint8_t high;
};

struct _Zone {
    Range keyRange;
    Range velocityRange;

    union {
        Instrument *instrument;
        SampleInfo *sampleInfo;
    };
};

struct _SampleInfo {
    bool loop;
    bool depression;

    Sample *L;
    Sample *R;
};

struct _Sample {
    const char *name;

    uint32_t start;
    uint32_t startLoop;
    uint32_t endLoop;
    uint32_t end;
    uint32_t sampleRate;
    uint8_t originalPitch;
    int8_t pitchCorrection;
};

bool ParsePresets(SoundFont *sf, Preset ***results, int *resultsCount);
void PresetsDump(Preset **presets);
void PresetDump(Preset *preset);
