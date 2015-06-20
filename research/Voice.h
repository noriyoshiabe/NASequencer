#pragma once

#include "SoundFont.h"
#include "Preset.h"
#include "AudioSample.h"

#define MAX_POLYPHONY 64

typedef enum {
    VolEnvPhaseDelay,
    VolEnvPhaseAttack,
    VolEnvPhaseHold,
    VolEnvPhaseDecay,
    VolEnvPhaseSustain,
    VolEnvPhaseRelease,
} VolEnvPhase;

typedef struct _Voice {
    uint8_t channel;
    uint8_t key;
    uint8_t velocity;

    uint32_t tick;
    float time;
    float sampleIncrement;
    float sampleIndex;

    VolEnvPhase phase;
    float startPhaseTime;
    float volEnv;
    float releasedVolEnv;

    Zone *presetGlobalZone;
    Zone *presetZone;
    Zone *instrumentGlobalZone;
    Zone *instrumentZone;

    SoundFont *sf;

    float sampleRate;
    int16_t sampleModes;
    uint8_t keyForSample;

    struct {
        int16_t delayVolEnv;
        int16_t attackVolEnv;
        int16_t holdVolEnv;
        int16_t decayVolEnv;
        int16_t sustainVolEnv;
        int16_t releaseVolEnv;
        int16_t keynumToVolEnvHold;
        int16_t keynumToVolEnvDecay;

        uint32_t sampleStartLoop;
        uint32_t sampleEndLoop;
        uint32_t sampleEnd;

        int16_t pan;
    } cache;

    struct _Voice *next;
    struct _Voice *prev;
} Voice;

typedef struct _VoicePool {
    Voice *buffer;
    Voice *freeList;
} VoicePool;

extern void VoiceInitialize(Voice *self, uint8_t channel, uint8_t noteNo, uint8_t velocity,
        Zone *presetGlobalZone, Zone *presetZone, Zone *instrumentGlobalZone, Zone *instrumentZone,
        SoundFont *sf, float sampleRate);

extern void VoiceUpdate(Voice *self);
extern AudioSample VoiceComputeSample(Voice *self);
extern void VoiceIncrementSample(Voice *self);
extern void VoiceRelease(Voice *self);
extern bool VoiceIsReleased(Voice *self);
extern void VoiceDump(Voice *voice);

extern VoicePool *VoicePoolCreate();
extern void VoicePoolDestroy(VoicePool *self);
extern Voice* VoicePoolAllocVoice(VoicePool *self);
extern void VoicePoolDeallocVoice(VoicePool *self, Voice *voice);
