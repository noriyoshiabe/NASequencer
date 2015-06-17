#include "Synthesizer.h"
#include "MidiSource.h"
#include "SoundFont.h"
#include "Preset.h"
#include "Voice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct _Synthesizer {
    MidiSource srcVtbl;

    char *filepath;
    SoundFont *sf;

    Preset **presets;
    int presetCount;

    uint32_t tick;

    VoicePool *voicePool;
    Voice *voiceList;
};

static Preset *SynthesizerFindPreset(Synthesizer *self, uint16_t midiPresetNo, uint16_t bankNo);
static int PresetComparator(const void *preset1, const void *preset2);
static int SynthesizerNoteOn(Synthesizer *self, uint8_t channel, uint8_t noteNo, uint8_t velocity);
static void SynthesizerAddVoice(Synthesizer *self, Voice *voice);
static void SynthesizerRemoveVoice(Synthesizer *self, Voice *voice);

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

    self->voicePool = VoicePoolCreate();

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
    for (Voice *voice = self->voiceList; NULL != voice; voice = voice->next) {
        VoiceDump(voice);
    }
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

    VoicePoolDestroy(self->voicePool);
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

    Preset *preset = SynthesizerFindPreset(self, 29, 0);
    //Preset *preset = self->presets[0]; // TODO from channel map
    
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

            Voice *voice = VoicePoolAllocVoice(self->voicePool);
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

            voice->sf = self->sf;

            voice->startTick = self->tick;

            SynthesizerAddVoice(self, voice);

            ++voicedCount;
        }
    }

END:
    return voicedCount;
}

static void SynthesizerAddVoice(Synthesizer *self, Voice *voice)
{
    if (self->voiceList) {
        self->voiceList->prev = voice;
    }

    voice->next = self->voiceList;
    self->voiceList = voice;
}

static void SynthesizerRemoveVoice(Synthesizer *self, Voice *voice)
{
    if (voice->prev) {
        voice->prev->next = voice->next;
    }

    if (voice->next) {
        voice->next->prev = voice->prev;
    }

    if (self->voiceList == voice) {
        self->voiceList = voice->next;
    }

    voice->prev = NULL;
    voice->next = NULL;
}

void SynthesizerComputeAudioSample(Synthesizer *self, uint32_t sampleRate, AudioSample *buffer, uint32_t count)
{
    // 7.10 The SHDR Sub-chunk
    // The values of dwStart, dwEnd, dwStartloop, and dwEndloop
    // must all be within the range of the sample data field
    // included in the SoundFont compatible bank or referenced in the sound ROM.
    // Also, to allow a variety of hardware platforms to be able to reproduce the data,
    // the samples have a minimum length of 48 data points,a minimum loop size of 32 data points
    // and a minimum of 8 valid points prior to dwStartloop and after dwEndloop.
    // Thus dwStart must be less than dwStartloop-7,

    for (Voice *voice = self->voiceList; NULL != voice; voice = voice->next) {
        uint32_t sampleStart = VoiceSampleStart(voice);
        uint32_t sampleEnd = VoiceSampleEnd(voice);
        // TODO looping
        //uint32_t sampleStartLoop = VoiceSampleStartLoop(voice);
        //uint32_t sampleEndLoop = VoiceSampleEndLoop(voice);
        
        for (int i = 0; i < count; ++i) {
            uint32_t current = sampleStart + (self->tick - voice->startTick) + i;

            // TODO pan
            // TODO 24bit sample
            buffer->L = (float)self->sf->smpl[current] / (float)SHRT_MAX;
            buffer->R = (float)self->sf->smpl[current] / (float)SHRT_MAX;

            // `<` insted of `<=` beacause, `a minimum of 8 valid points prior to dwStartloop and after dwEndloop.`
            if (sampleEnd - sampleStart < current) {
                Voice *toFree = voice;
                voice = voice->next;
                SynthesizerRemoveVoice(self, toFree);
                VoicePoolDeallocVoice(self->voicePool, toFree);
                break;
            }
        }
    }

    self->tick += count;
}
