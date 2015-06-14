#pragma once

#include "SoundFont.h"
#include <stdbool.h>

typedef struct _Preset Preset;
typedef struct _Instrument Instrument;
typedef struct _Sample Sample;
typedef struct _Range Range;
typedef struct _Zone Zone;
typedef struct _Generator Generator;

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

struct _Sample {
    const char *name;

    uint32_t start;
    uint32_t startLoop;
    uint32_t endLoop;
    uint32_t end;
    uint32_t sampleRate;
    uint8_t originalPitch;
    int8_t pitchCorrection;

    uint16_t sampleType;
};

struct _Range {
    uint8_t low;
    uint8_t high;
} __attribute__((__packed__));

struct _Generator {
    int64_t flags;

    union {
        int16_t array[SFGeneratorType_endOper];

        struct {
            int16_t startAddrsOffset;
            int16_t endAddrsOffset;
            int16_t startloopAddrsOffset;
            int16_t endloopAddrsOffset;
            int16_t startAddrsCoarseOffset;
            int16_t modLfoToPitch;
            int16_t vibLfoToPitch;
            int16_t modEnvToPitch;
            int16_t initialFilterFc;
            int16_t initialFilterQ;
            int16_t modLfoToFilterFc;
            int16_t modEnvToFilterFc;
            int16_t endAddrsCoarseOffset;
            int16_t modLfoToVolume;
            int16_t unused1;
            int16_t chorusEffectsSend;
            int16_t reverbEffectsSend;
            int16_t pan;
            int16_t unused2;
            int16_t unused3;
            int16_t unused4;
            int16_t delayModLFO;
            int16_t freqModLFO;
            int16_t delayVibLFO;
            int16_t freqVibLFO;
            int16_t delayModEnv;
            int16_t attackModEnv;
            int16_t holdModEnv;
            int16_t decayModEnv;
            int16_t sustainModEnv;
            int16_t releaseModEnv;
            int16_t keynumToModEnvHold;
            int16_t keynumToModEnvDecay;
            int16_t delayVolEnv;
            int16_t attackVolEnv;
            int16_t holdVolEnv;
            int16_t decayVolEnv;
            int16_t sustainVolEnv;
            int16_t releaseVolEnv;
            int16_t keynumToVolEnvHold;
            int16_t keynumToVolEnvDecay;
            uint16_t instrument;
            int16_t reserved1;
            Range keyRange;
            Range velRange;
            int16_t startloopAddrsCoarseOffset;
            int16_t keynum;
            int16_t velocity;
            int16_t initialAttenuation;
            int16_t reserved2;
            int16_t endloopAddrsCoarseOffset;
            int16_t coarseTune;
            int16_t fineTune;
            uint16_t sampleID;
            int16_t sampleModes;
            int16_t reserved3;
            int16_t scaleTuning;
            int16_t exclusiveClass;
            int16_t overridingRootKey;
            int16_t unused5;
        };
    };
};

struct _Zone {
    Generator gen;
    Instrument *instrument;
    Sample *sample;
};

extern bool ParsePresets(SoundFont *sf, Preset ***results, int *resultsCount);
extern void PresetDestroy(Preset *self);
extern void PresetDump(Preset *preset);
extern void InstrumentDump(Instrument *instrument);
extern void SampleDump(Sample *sample);
extern bool ZoneHasGenerator(Zone *zone, SFGeneratorType generatorType);
extern int GeneratorDefaultValue(SFGeneratorType generatorType);
