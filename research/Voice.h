#pragma once

#include "SoundFont.h"
#include "Preset.h"
#include "AudioSample.h"
#include "IIRFilter.h"

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
    double time;
    double sampleIncrement;
    double sampleIndex;

    VolEnvPhase phase;
    double startPhaseTime;
    double volEnv;
    double releasedVolEnv;

    Zone *presetGlobalZone;
    Zone *presetZone;
    Zone *instrumentGlobalZone;
    Zone *instrumentZone;

    SoundFont *sf;

    double sampleRate;
    int16_t sampleModes;
    uint8_t keyForSample;

    uint8_t exclusiveClass;
    uint8_t terminated;

    IIRFilter LPF;

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
        SoundFont *sf, double sampleRate);

extern void VoiceUpdate(Voice *self);
extern AudioSample VoiceComputeSample(Voice *self);
extern void VoiceIncrementSample(Voice *self);
extern void VoiceRelease(Voice *self);
extern void VoiceTerminate(Voice *self);
extern bool VoiceIsReleased(Voice *self);
extern void VoiceDump(Voice *voice);

extern VoicePool *VoicePoolCreate();
extern void VoicePoolDestroy(VoicePool *self);
extern Voice* VoicePoolAllocVoice(VoicePool *self);
extern void VoicePoolDeallocVoice(VoicePool *self, Voice *voice);
