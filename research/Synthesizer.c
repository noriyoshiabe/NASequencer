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
};

static Preset *SynthesizerFindPreset(Synthesizer *self, uint16_t midiPresetNo, uint16_t bankNo);
static int PresetComparator(const void *preset1, const void *preset2);
static int SynthesizerNoteOn(Synthesizer *self, uint8_t channel, uint8_t noteNo, uint8_t velocity);

static Voice voicePool[MAX_POLYPHONY];
static Voice *voiceList = NULL;
static Voice *voiceFreeList = NULL;

static Voice *VoiceAlloc();
static void VoiceDealloc(Voice *voice);
static void VoiceDump(Voice *voice);
static void VoiceListDump();

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
    VoiceListDump();
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

            Voice *voice = VoiceAlloc();
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

static Voice *VoiceAlloc()
{
    if (!voiceFreeList) {
        return NULL;
    }

    Voice *ret = voiceFreeList;
    voiceFreeList = ret->next;

    memset(ret, 0, sizeof(Voice));

    if (voiceList) {
        voiceList->prev = ret;
    }

    ret->next = voiceList;
    ret->prev = NULL;
    voiceList = ret;

    return ret;
}

static void VoiceDealloc(Voice *voice)
{
    if (voice->prev) {
        voice->prev->next = voice->next;
    }

    if (voice->next) {
        voice->next->prev = voice->prev;
    }

    voice->prev = NULL;
    voice->next = voiceFreeList;
    voiceFreeList = voice;
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

static void VoiceListDump()
{
    for (Voice *voice = voiceList; NULL != voice; voice = voice->next) {
        VoiceDump(voice);
    }
}

static void __attribute__((constructor)) initializeVoiceFreeList()
{
    voiceFreeList = &voicePool[0];

    Voice *prev = NULL;

    for (int i = 0; i < MAX_POLYPHONY; ++i) {
        if (prev) {
            prev->next = &voicePool[i];
        }

        voicePool[i].prev = prev;
        prev = &voicePool[i];
    }

    voicePool[MAX_POLYPHONY - 1].next = NULL;
}
