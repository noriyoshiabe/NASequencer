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
    VolEnvPhaseRelase,
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

    struct _Voice *next;
    struct _Voice *prev;
} Voice;

typedef struct _VoicePool {
    Voice *buffer;
    Voice *freeList;
} VoicePool;

extern uint32_t VoiceSampleStart(Voice *self);
extern uint32_t VoiceSampleEnd(Voice *self);
extern uint32_t VoiceSampleStartLoop(Voice *self);
extern uint32_t VoiceSampleEndLoop(Voice *self);

extern void VoiceUpdate(Voice *self, uint32_t sampleRate);
extern AudioSample VoiceComputeSample(Voice *self);
extern void VoiceIncrementSample(Voice *self);
extern void VoiceRelease(Voice *self);
extern bool VoiceIsReleased(Voice *self);

extern int16_t VoiceGeneratorShortValue(Voice *self, SFGeneratorType generatorType);

extern void VoiceDump(Voice *voice);

extern VoicePool *VoicePoolCreate();
extern void VoicePoolDestroy(VoicePool *self);
extern Voice* VoicePoolAllocVoice(VoicePool *self);
extern void VoicePoolDeallocVoice(VoicePool *self, Voice *voice);
