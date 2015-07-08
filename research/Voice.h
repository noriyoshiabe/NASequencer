#pragma once

#include "SoundFont.h"
#include "Preset.h"
#include "AudioSample.h"
#include "IIRFilter.h"
#include "ADSREnvelope.h"
#include "LFO.h"

#define MAX_POLYPHONY 64

typedef struct _Voice {
    uint8_t channel;
    uint8_t key;
    uint8_t velocity;

    uint32_t tick;
    double sampleIncrement;
    double sampleIndex;

    Zone *presetGlobalZone;
    Zone *presetZone;
    Zone *instrumentGlobalZone;
    Zone *instrumentZone;

    SoundFont *sf;

    double sampleRate;
    int16_t sampleModes;
    uint8_t keyForSample;

    uint8_t exclusiveClass;

    IIRFilter LPF;
    ADSREnvelope volEnv;
    ADSREnvelope modEnv;
    LFO vibLfo;
    LFO modLfo;

    uint32_t sampleStartLoop;
    uint32_t sampleEndLoop;
    uint32_t sampleEnd;

    int16_t pan;
    int16_t initialAttenuation;

    int16_t modLfoToPitch;
    int16_t vibLfoToPitch;
    int16_t modEnvToPitch;

    int16_t initialFilterFc;
    int16_t initialFilterQ;
    int16_t modLfoToFilterFc;

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
