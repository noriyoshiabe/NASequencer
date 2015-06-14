#pragma once

#include "SoundFont.h"
#include <stdbool.h>

typedef struct _Preset Preset;
typedef struct _Instrument Instrument;
typedef struct _Sample Sample;
typedef struct _Zone Zone;
typedef struct _RangeGenerator RangeGenerator;
typedef struct _SubstitutionGenerator SubstitutionGenerator;
typedef struct _SampleGenerator SampleGenerator;
typedef struct _ValueGenerator ValueGenerator;

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
};

struct _RangeGenerator {
    struct {
        uint8_t low;
        uint8_t high;
    } key;
    struct {
        uint8_t low;
        uint8_t high;
    } velocity;
};

struct _SubstitutionGenerator {
    int8_t keynum;
    int8_t velocity;
};

struct _SampleGenerator {
    int16_t startAddrsOffset;
    int16_t endAddrsOffset;
    int16_t startloopAddrsOffset;
    int16_t endloopAddrsOffset;

    uint16_t startAddrsCoarseOffset;
    uint16_t endAddrsCoarseOffset;
    uint16_t startloopAddrsCoarseOffset;
    uint16_t endloopAddrsCoarseOffset;

    int8_t exclusiveClass;
    int8_t overridingRootKey;

    bool loop;
    bool untilRelease;

    Sample *L;
    Sample *R;
};

struct _ValueGenerator {
    int16_t modLfoToPitch;
    int16_t vibLfoToPitch;
    int16_t modEnvToPitch;
    int16_t initialFilterFc;
    int16_t initialFilterQ;
    int16_t modLfoToFilterFc;
    int16_t modEnvToFilterFc;
    int16_t modLfoToVolume;
    int16_t chorusEffectsSend;
    int16_t reverbEffectsSend;
    int16_t pan;
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
    int16_t initialAttenuation;
    int16_t coarseTune;
    int16_t fineTune;
    int16_t scaleTuning;
};

struct _Zone {
    RangeGenerator range;
    SubstitutionGenerator substitution;
    ValueGenerator value;

    Instrument *instrument;
    SampleGenerator sample;
};

extern bool ParsePresets(SoundFont *sf, Preset ***results, int *resultsCount);
extern void PresetDestroy(Preset *self);
extern void PresetDump(Preset *preset);
