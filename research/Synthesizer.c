#include "Synthesizer.h"
#include "MidiSource.h"
#include "SoundFont.h"
#include "Preset.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    struct _Voice *next;
    struct _Voice *prev;
} Voice;

struct _Synthesizer {
    MidiSource srcVtbl;

    char *filepath;
    SoundFont *sf;

    Preset **presets;
    int presetCount;

    Voice *voicePool;
    Voice *voiceList;
    Voice *voiceFreeList;
};

static Preset *SynthesizerFindPreset(Synthesizer *self, uint16_t midiPresetNo, uint16_t bankNo);
static int PresetComparator(const void *preset1, const void *preset2);
static int SynthesizerNoteOn(Synthesizer *self, uint8_t channel, uint8_t noteNo, uint8_t velocity);

static void SynthesizerInitializeVoiceFreeList(Synthesizer *self);
static Voice *SynthesizerVoiceAlloc(Synthesizer *self);
static void SynthesizerVoiceDealloc(Synthesizer *self, Voice *voice);
static void VoiceDump(Voice *voice);
static void SynthesizerVoiceListDump(Synthesizer *self);

static void send(void *self, uint8_t *bytes, size_t length)
{
}

static bool isAvailable(void *self)
{
    return true;
}

static bool hasProperty(void *self, const char *name)
{
    return false;
}

static void setProperty(void *self, const char *name, const void *value)
{
}

static void getProperty(void *self, const char *name, void *value)
{
}

Synthesizer *SynthesizerCreate(const char *filepath)
{
    Synthesizer *self = calloc(1, sizeof(Synthesizer));

    self->srcVtbl.send = send;
    self->srcVtbl.isAvailable = isAvailable;
    self->srcVtbl.hasProperty = hasProperty;
    self->srcVtbl.setProperty = setProperty;
    self->srcVtbl.getProperty = getProperty;

    self->filepath = strdup(filepath);
    self->sf = SoundFontRead(filepath, NULL);

    SynthesizerInitializeVoiceFreeList(self);

    ParsePresets(self->sf, &self->presets, &self->presetCount);
    qsort(self->presets, self->presetCount, sizeof(Preset *), PresetComparator);

#if 0
    for (int i = 0; i < self->presetCount; ++i) {
        PresetDump(self->presets[i]);
    }
#endif

#if 0
    Preset *preset = SynthesizerFindPreset(self, 25, 128);
    //Preset *preset = self->presets[0];
    PresetDump(preset);

    for (int i = 0; i < preset->zoneCount; ++i) {
        InstrumentDump(preset->zones[i]->instrument);
    }

    for (int i = 0; i < preset->zoneCount; ++i) {
        for (int j = 0; j < preset->zones[i]->instrument->zoneCount; ++j) {
            SampleDump(preset->zones[i]->instrument->zones[j]->sample);
        }
    }
#endif

#if 1
    SynthesizerNoteOn(self, 0, 64, 127);
    SynthesizerVoiceListDump(self);
#endif

    return self;
}

void SynthesizerDestroy(Synthesizer *self)
{
    for (int i = 0; i < self->presetCount; ++i) {
        PresetDestroy(self->presets[i]);
    }

    if (self->presets) {
        free(self->presets);
    }

    free(self->voicePool);

    SoundFontDestroy(self->sf);

    free(self->filepath);
    free(self);
}

static Preset *SynthesizerFindPreset(Synthesizer *self, uint16_t midiPresetNo, uint16_t bankNo)
{
    Preset preset = { .midiPresetNo = midiPresetNo, .bankNo = bankNo };
    Preset *key = &preset;
    Preset **result = bsearch(&key, self->presets, self->presetCount, sizeof(Preset *), PresetComparator);
    return result ? *result : NULL;
}

static int PresetComparator(const void *_preset1, const void *_preset2)
{
    const Preset **preset1 = (const Preset **)_preset1;
    const Preset **preset2 = (const Preset **)_preset2;

    return (((*preset1)->bankNo << 16) | (*preset1)->midiPresetNo) - (((*preset2)->bankNo << 16) | (*preset2)->midiPresetNo);
}

static int SynthesizerNoteOn(Synthesizer *self, uint8_t channel, uint8_t noteNo, uint8_t velocity)
{
    int voicedCount = 0;

    Preset *preset = self->presets[0]; // TODO from channel map
    
    for (int i = 0; i < preset->zoneCount; ++i) {
        Zone *presetZone = preset->zones[i];

        if (!ZoneIsInsideRange(presetZone, noteNo, velocity)) {
            continue;
        }

        Instrument *instrument = presetZone->instrument;
        for (int j = 0; j < instrument->zoneCount; ++j) {
            Zone *instrumentZone = instrument->zones[j];

            if (!ZoneIsInsideRange(instrumentZone, noteNo, velocity)) {
                continue;
            }

            Voice *voice = SynthesizerVoiceAlloc(self);
            if (!voice) {
                goto END;
            }

            voice->channel = channel;
            voice->key = noteNo;
            voice->velocity = velocity;

            voice->presetGlobalZone = preset->globalZone;
            voice->presetZone = presetZone;
            voice->instrumentGlobalZone = instrument->globalZone;
            voice->instrumentZone = instrumentZone;

            // TODO record current time to startedAt
            
            ++voicedCount;
        }
    }

END:
    return voicedCount;
}

static void SynthesizerInitializeVoiceFreeList(Synthesizer *self)
{
    self->voicePool = calloc(MAX_POLYPHONY, sizeof(Voice));
    self->voiceFreeList = &self->voicePool[0];

    Voice *prev = NULL;

    for (int i = 0; i < MAX_POLYPHONY; ++i) {
        if (prev) {
            prev->next = &self->voicePool[i];
        }

        self->voicePool[i].prev = prev;
        prev = &self->voicePool[i];
    }

    self->voicePool[MAX_POLYPHONY - 1].next = NULL;
}

static Voice *SynthesizerVoiceAlloc(Synthesizer *self)
{
    if (!self->voiceFreeList) {
        return NULL;
    }

    Voice *ret = self->voiceFreeList;
    self->voiceFreeList = ret->next;

    memset(ret, 0, sizeof(Voice));

    if (self->voiceList) {
        self->voiceList->prev = ret;
    }

    ret->next = self->voiceList;
    ret->prev = NULL;
    self->voiceList = ret;

    return ret;
}

static void SynthesizerVoiceDealloc(Synthesizer *self, Voice *voice)
{
    if (voice->prev) {
        voice->prev->next = voice->next;
    }

    if (voice->next) {
        voice->next->prev = voice->prev;
    }

    voice->prev = NULL;
    voice->next = self->voiceFreeList;
    self->voiceFreeList = voice;
}

static void VoiceDump(Voice *voice)
{
    printf("[Voice]\n");
    printf("-----------------------------------------\n");
    printf("channel: %d\n", voice->channel);
    printf("key: %d\n", voice->key);
    printf("velocity: %d\n", voice->velocity);
    printf("startedAt: %lld\n", voice->startedAt);

    if (voice->presetGlobalZone) {
        printf("presetGlobalZone:\n");
        ZoneDump(voice->presetGlobalZone);
    }

    printf("presetZone:\n");
    ZoneDump(voice->presetZone);

    if (voice->instrumentGlobalZone) {
        printf("instrumentGlobalZone:\n");
        ZoneDump(voice->instrumentGlobalZone);
    }

    printf("instrumentZone:\n");
    ZoneDump(voice->instrumentZone);

    printf("\n");
}

static void SynthesizerVoiceListDump(Synthesizer *self)
{
    for (Voice *voice = self->voiceList; NULL != voice; voice = voice->next) {
        VoiceDump(voice);
    }
}
