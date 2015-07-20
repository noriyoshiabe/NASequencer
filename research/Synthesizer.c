#include "Synthesizer.h"
#include "MidiSource.h"
#include "Preset.h"
#include "Voice.h"
#include "Chorus.h"
#include "Reverb.h"
#include "Channel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHANNEL_COUNT 16

typedef struct _Callback {
    void *receiver;
    MidiSourceCallback function;
} Callback;

struct _Synthesizer {
    MidiSource srcVtbl;

    SoundFont *sf;

    Preset **presets;
    int presetCount;

    uint32_t tick;

    VoicePool *voicePool;
    Voice *voiceFirst;
    Voice *voiceLast;
    int voicingCount;

    double sampleRate;

    Chorus *chorus;
    Reverb *reverb;

    Channel channels[CHANNEL_COUNT];

    Callback *callbackList;
    int32_t callbackListLength;
};

static Preset *SynthesizerFindPreset(Synthesizer *self, uint16_t midiPresetNo, uint16_t bankNo);
static int PresetComparator(const void *preset1, const void *preset2);
static int SynthesizerNoteOn(Synthesizer *self, uint8_t channel, uint8_t noteNo, uint8_t velocity);
static void SynthesizerNoteOff(Synthesizer *self, uint8_t channel, uint8_t noteNo);
static void SynthesizerReleaseExclusiveClass(Synthesizer *self, Voice *newVoice);
static void SynthesizerProgramChange(Synthesizer *self, uint8_t channel, uint8_t programNo);
static void SynthesizerControlChange(Synthesizer *self, uint8_t channel, uint8_t ccNumber, uint8_t value);
static void SynthesizerAddVoice(Synthesizer *self, Voice *voice);
static void SynthesizerRemoveVoice(Synthesizer *self, Voice *voice);

static void send(void *_self, uint8_t *bytes, size_t length)
{
    Synthesizer *self = _self;

    switch (bytes[0] & 0xF0) {
    case 0x90:
        if (3 <= length) {
            SynthesizerNoteOn(self, bytes[0] & 0x0F, bytes[1], bytes[2]);
        }
        break;
    case 0x80:
        if (3 <= length) {
            SynthesizerNoteOff(self, bytes[0] & 0x0F, bytes[1]);
        }
        break;
    case 0xA0:
        if (3 <= length) {
            uint8_t channel = bytes[0] & 0x0F;
            self->channels[channel].keyPressure[0x7F & bytes[1]] = bytes[2];
        }
        break;
    case 0xB0:
        if (3 <= length) {
            uint8_t channel = bytes[0] & 0x0F;
            SynthesizerControlChange(self, channel, bytes[1], bytes[2]);
        }
        break;
    case 0xC0:
        if (2 <= length) {
            uint8_t channel = bytes[0] & 0x0F;
            SynthesizerProgramChange(self, channel, bytes[1]);
        }
        break;
    case 0xD0:
        if (2 <= length) {
            uint8_t channel = bytes[0] & 0x0F;
            self->channels[channel].channelPressure = bytes[1];
        }
        break;
    }
}

static bool isAvailable(void *_self)
{
    Synthesizer *self = _self;
    return self->sf && 0 < self->presetCount;
}

static void registerCallback(void *self, MidiSourceCallback callback, void *receiver)
{
    self->callbackList = realloc(self->callbackList, sizeof(Callback) * self->callbackListLength + 1);
    self->callbackList[self->callbackListLength].function = function; 
    self->callbackList[self->callbackListLength].receiver = receiver; 
    ++self->callbackListLength;
}

static void ungisterCallback(void *self, MidiSourceCallback callback, void *receiver)
{
    for (int i = 0; i < self->callbackListLength; ++i) {
        if (self->callbackList[i].function == function
                && self->callbackList[i].receiver == receiver) {

            size_t moveLength = self->callbackListLength - 1 - i;
            if (0 < moveLength) {
                memmove(&self->callbackList[i + 1], &self->callbackList[i], sizeof(Callback) * moveLength);
            }

            --self->callbackListLength;
            self->callbackList = realloc(self->callbackList, sizeof(Callback) * self->callbackListLength);
            break;
        }
    }
}

static const char *getName(void *self)
{
    return ((Synthesizer *)self)->sf->INAM;
}

static int getPresetCount(void *self)
{
    return ((Synthesizer *)self)->presetCount;
}

static void getPresetList(void *_self, PresetList *presetList, size_t count)
{
    Synthesizer *self = _self;

    for (int i = 0; i < self->presetCount; ++i) {
        strncpy(presetList[i].name, self->presets[i]->name, sizeof(presetList[i].name));
        presetList[i].bankNo = self->presets[i]->bankNo;
        presetList[i].programNo = self->presets[i]->midiPresetNo;
    }
}

static int getPresetIndex(void *_self, uint8_t channel)
{
    Synthesizer *self = _self;
    Preset **result = bsearch(self->channels[channel].preset, self->presets, self->presetCount, sizeof(Preset *), PresetComparator);
    return result - self->presets;
}

static void setPresetIndex(void *_self, uint8_t channel, int index)
{
    Synthesizer *self = _self;
    SynthesizerProgramChange(self, channel, self->presets[index]->midiPresetNo);
}

static int16_t getLastMasterLevel(void *self) { return 0; }
static int16_t getLastLevel(void *self, uint8_t channel) { return 0; }
static void setMasterVolume(void *self, int16_t value) { }

static void setVolume(void *self, uint8_t channel, uint8_t value)
{
    SynthesizerControlChange(self, channel, CC_Volume_MSB, value);
}

static void setPan(void *self, uint8_t channel, uint8_t value)
{
    SynthesizerControlChange(self, channel, CC_Pan_MSB, value);
}

static void setChorusSend(void *self, uint8_t channel, uint8_t value)
{
    SynthesizerControlChange(self, channel, CC_Effect3Depth, value);
}

static void setReverbSend(void *self, uint8_t channel, uint8_t value)
{
    SynthesizerControlChange(self, channel, CC_Effect1Depth, value);
}

static int16_t getMasterVolume(void *self) { return 0; }

static uint8_t getVolume(void *self, uint8_t channel)
{
    return ((Synthesizer *)self)->channels[channel].cc[CC_Volume_MSB];
}

static uint8_t getPan(void *self, uint8_t channel)
{
    return ((Synthesizer *)self)->channels[channel].cc[CC_Pan_MSB];
}

static uint8_t getChorusSend(void *self, uint8_t channel)
{
    return ((Synthesizer *)self)->channels[channel].cc[CC_Effect3Depth];
}

static uint8_t getReverbSend(void *self, uint8_t channel)
{
    return ((Synthesizer *)self)->channels[channel].cc[CC_Effect1Depth];
}

Synthesizer *SynthesizerCreate(SoundFont *sf, double sampleRate)
{
    Synthesizer *self = calloc(1, sizeof(Synthesizer));

    self->srcVtbl.send = send;
    self->srcVtbl.isAvailable = isAvailable;
    self->srcVtbl.registerCallback = registerCallback;
    self->srcVtbl.ungisterCallback = ungisterCallback;
    self->srcVtbl.getName = getName;
    self->srcVtbl.getPresetCount = getPresetCount;
    self->srcVtbl.getPresetList = getPresetList;
    self->srcVtbl.getPresetIndex = getPresetIndex;
    self->srcVtbl.setPresetIndex = setPresetIndex;
    self->srcVtbl.getLastMasterLevel = getLastMasterLevel;
    self->srcVtbl.getLastLevel = getLastLevel;
    self->srcVtbl.setMasterVolume = setMasterVolume;
    self->srcVtbl.setVolume = setVolume;
    self->srcVtbl.setPan = setPan;
    self->srcVtbl.setChorusSend = setChorusSend;
    self->srcVtbl.setReverbSend = setReverbSend;
    self->srcVtbl.getMasterVolume = getMasterVolume;
    self->srcVtbl.getVolume = getVolume;
    self->srcVtbl.getPan = getPan;
    self->srcVtbl.getChorusSend = getChorusSend;
    self->srcVtbl.getReverbSend = getReverbSend;

    self->sf = sf;

    self->sampleRate = sampleRate;

    self->voicePool = VoicePoolCreate();

    self->chorus = ChorusCreate(sampleRate);

    ChorusAddDelay(self->chorus, 0.020, 1.00, 0.001, 0.7, 0.7);
    ChorusAddDelay(self->chorus, 0.020, 0.50, 0.001, 1.0, 0.0);
    ChorusAddDelay(self->chorus, 0.020, 0.25, 0.001, 0.0, 1.0);

    self->reverb = ReverbCreate(sampleRate, 1.0);

    ParsePresets(self->sf, &self->presets, &self->presetCount);
    qsort(self->presets, self->presetCount, sizeof(Preset *), PresetComparator);

    if (0 < self->presetCount) {
        for (int i = 0; i < CHANNEL_COUNT; ++i) {
            ChannelInitialize(&self->channels[i], i, self->presets[0]);
        }
    }

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

    if (self->callbackList) {
        free(self->callbackList);
    }

    ChorusDestroy(self->chorus);
    ReverbDestroy(self->reverb);

    VoicePoolDestroy(self->voicePool);
    SoundFontDestroy(self->sf);

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

    if (CHANNEL_COUNT <= channel) {
        return voicedCount;
    }

    Preset *preset = self->channels[channel].preset;
    
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

            VoiceInitialize(voice, &self->channels[channel], noteNo, velocity,
                    preset->globalZone, presetZone, instrument->globalZone, instrumentZone,
                    self->sf, self->sampleRate);

            SynthesizerReleaseExclusiveClass(self, voice);

            SynthesizerAddVoice(self, voice);

#if 0
            VoiceDump(voice);
#endif

            ++voicedCount;
        }
    }

END:
    return voicedCount;
}

static void SynthesizerNoteOff(Synthesizer *self, uint8_t channel, uint8_t noteNo)
{
    for (Voice *voice = self->voiceFirst; NULL != voice; voice = voice->next) {
        if (voice->channel == &self->channels[channel]
                && voice->preset == self->channels[channel].preset
                && voice->key == noteNo) {

            if (ChannelIsSustained(&self->channels[channel])) {
                voice->sustain = true;
            }
            else {
                VoiceRelease(voice);
            }
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

    for (Voice *voice = self->voiceFirst; NULL != voice; voice = voice->next) {
        if (voice->channel == newVoice->channel
                && voice->preset == newVoice->preset
                && voice->exclusiveClass == newVoice->exclusiveClass) {
            VoiceTerminate(voice);
        }
    }
}

static void SynthesizerControlChange(Synthesizer *self, uint8_t channel, uint8_t ccNumber, uint8_t value)
{
    ChannelSetControlChange(&self->channels[channel], ccNumber, value);

    for (Voice *voice = self->voiceFirst; NULL != voice; voice = voice->next) {
        if (voice->channel == &self->channels[channel]) {
            VoiceUpdateRuntimeParams(voice);

            switch (ccNumber) {
            case CC_Sustain:
                if (voice->sustain && !ChannelIsSustained(voice->channel)) {
                    VoiceRelease(voice);
                }
                break;
            case CC_AllSoundOff:
                VoiceTerminate(voice);
                break;
            case CC_AllNotesOff:
                if (ChannelIsSustained(voice->channel)) {
                    voice->sustain = true;
                }
                else {
                    VoiceRelease(voice);
                }
                break;
            }
        }
    }
}

static void SynthesizerProgramChange(Synthesizer *self, uint8_t channel, uint8_t programNo)
{
    uint16_t bankNo = ChannelGetBankNumber(&self->channels[channel]);
    Preset *preset = SynthesizerFindPreset(self, programNo, bankNo);
    if (preset) {
        self->channels[channel].preset = preset;
#if 0
        PresetDump(preset);
#endif
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
    for (int i = 0; i < count; ++i) {
        AudioSample direct = { .L = 0.0, .R = 0.0 };
        AudioSample chorus = { .L = 0.0, .R = 0.0 };
        AudioSample reverb = { .L = 0.0, .R = 0.0 };

        for (Voice *voice = self->voiceFirst; NULL != voice;) {
            VoiceUpdate(voice);

            AudioSample sample = VoiceComputeSample(voice);
            direct.L += sample.L;
            direct.R += sample.R;

            double chorusSend = VoiceChorusEffectsSend(voice);
            chorus.L += sample.L * chorusSend;
            chorus.R += sample.R * chorusSend;

            double reverbSend = VoiceReverbEffectsSend(voice);
            reverb.L += sample.L * reverbSend;
            reverb.R += sample.R * reverbSend;

            VoiceIncrementSample(voice);

            if (VoiceIsReleased(voice)) {
                Voice *toFree = voice;
                voice = voice->next;
                SynthesizerRemoveVoice(self, toFree);
                VoicePoolDeallocVoice(self->voicePool, toFree);
                continue;
            }

            voice = voice->next;
        }

        chorus = ChorusComputeSample(self->chorus, chorus);
        reverb = ReverbComputeSample(self->reverb, reverb);

        buffer[i].L += direct.L + chorus.L + reverb.L;
        buffer[i].R += direct.R + chorus.R + reverb.R;
    }

    self->tick += count;
}

int SynthesizerVoicingCount(Synthesizer *self)
{
    return self->voicingCount;
}
