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
    Voice *voiceFirst;
    Voice *voiceLast;
    int voicingCount;

    float sampleRate;
};

static Preset *SynthesizerFindPreset(Synthesizer *self, uint16_t midiPresetNo, uint16_t bankNo);
static int PresetComparator(const void *preset1, const void *preset2);
static int SynthesizerNoteOn(Synthesizer *self, uint8_t channel, uint8_t noteNo, uint8_t velocity);
static void SynthesizerNoteOff(Synthesizer *self, uint8_t channel, uint8_t noteNo);
static void SynthesizerReleaseExclusiveClass(Synthesizer *self, Voice *newVoice);
static void SynthesizerAddVoice(Synthesizer *self, Voice *voice);
static void SynthesizerRemoveVoice(Synthesizer *self, Voice *voice);

static void send(void *self, uint8_t *bytes, size_t length)
{
#if 1
    if (0 == length) {
        SynthesizerNoteOn(self, 0, 64, 127);
    }
    else {
        SynthesizerNoteOff(self, 0, 64);
    }
#endif
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

Synthesizer *SynthesizerCreate(const char *filepath, float sampleRate)
{
    Synthesizer *self = calloc(1, sizeof(Synthesizer));

    self->srcVtbl.send = send;
    self->srcVtbl.isAvailable = isAvailable;
    self->srcVtbl.hasProperty = hasProperty;
    self->srcVtbl.setProperty = setProperty;
    self->srcVtbl.getProperty = getProperty;

    self->filepath = strdup(filepath);
    self->sf = SoundFontRead(filepath, NULL);

    self->sampleRate = sampleRate;

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

#if 0
    SynthesizerNoteOn(self, 0, 64, 127);
    for (Voice *voice = self->voiceFirst; NULL != voice; voice = voice->next) {
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

            VoiceInitialize(voice, channel, noteNo, velocity,
                    preset->globalZone, presetZone, instrument->globalZone, instrumentZone,
                    self->sf, self->sampleRate);

            SynthesizerReleaseExclusiveClass(self, voice);

            SynthesizerAddVoice(self, voice);

            ++voicedCount;
        }
    }

END:
    return voicedCount;
}

static void SynthesizerNoteOff(Synthesizer *self, uint8_t channel, uint8_t noteNo)
{
    for (Voice *voice = self->voiceFirst; NULL != voice; voice = voice->next) {
        if (voice->channel == channel && voice->key == noteNo) {
            VoiceRelease(voice);
            break;
        }
    }
}

static void SynthesizerReleaseExclusiveClass(Synthesizer *self, Voice *newVoice)
{
    // 8.1.1 Kinds of Generator Enumerators
    // 57 exclusiveClass
    // An exclusive class value of zero indicates no exclusive class; no special action is taken.
    if (0 == newVoice->exclusiveClass) {
        return;
    }

    // TODO check preset identity insted of channel

    for (Voice *voice = self->voiceFirst; NULL != voice; voice = voice->next) {
        if (voice->channel == newVoice->channel && voice->exclusiveClass == newVoice->exclusiveClass) {
            VoiceTerminate(voice);
        }
    }
}

static void SynthesizerAddVoice(Synthesizer *self, Voice *voice)
{
    if (!self->voiceFirst) {
        self->voiceFirst = voice;
        voice->prev= NULL;
    }

    if (self->voiceLast) {
        self->voiceLast->next = voice;
        voice->prev = self->voiceLast;
    }

    self->voiceLast = voice;

    voice->next = NULL;

    ++self->voicingCount;
}

static void SynthesizerRemoveVoice(Synthesizer *self, Voice *voice)
{
    if (voice->prev) {
        voice->prev->next = voice->next;
    }

    if (voice->next) {
        voice->next->prev = voice->prev;
    }

    if (self->voiceFirst == voice) {
        self->voiceFirst = voice->next;
    }

    if (self->voiceLast == voice) {
        self->voiceLast = voice->prev;
    }

    voice->prev = NULL;
    voice->next = NULL;

    --self->voicingCount;
}

void SynthesizerComputeAudioSample(Synthesizer *self, AudioSample *buffer, uint32_t count)
{
    for (Voice *voice = self->voiceFirst; NULL != voice;) {
        for (int i = 0; i < count; ++i) {
            VoiceUpdate(voice);

            AudioSample sample = VoiceComputeSample(voice);
            buffer[i].L += sample.L;
            buffer[i].R += sample.R;

            VoiceIncrementSample(voice);

            if (VoiceIsReleased(voice)) {
                Voice *toFree = voice;
                voice = voice->next;
                SynthesizerRemoveVoice(self, toFree);
                VoicePoolDeallocVoice(self->voicePool, toFree);
                goto NEXT;
            }
        }

        voice = voice->next;
NEXT:
        ;
    }

    self->tick += count;
}

int SynthesizerVoicingCount(Synthesizer *self)
{
    return self->voicingCount;
}
