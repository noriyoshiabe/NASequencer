#pragma once

#include "SoundFont.h"
#include "Preset.h"

#define MAX_POLYPHONY 64

typedef struct _Voice {
    uint8_t channel;
    uint8_t key;
    uint8_t velocity;

    int64_t startedAt;

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
    Voice *voiceList;
    Voice *voiceFreeList;
} VoicePool;

extern void VoiceDump(Voice *voice);

extern VoicePool *VoicePoolCreate();
extern void VoicePoolDestroy(VoicePool *self);
extern Voice* VoicePoolAllocVoice(VoicePool *self);
extern void VoicePoolDeallocVoice(VoicePool *self, Voice *voice);
extern void VoicePoolDump(VoicePool *self);
