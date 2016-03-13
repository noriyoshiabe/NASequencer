#include "Synthesizer.h"
#include "MidiSource.h"
#include "AudioSample.h"
#include "Preset.h"
#include "Voice.h"
#include "Channel.h"
#include "Define.h"

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

    PresetInfo **presetInfos;
    Preset **presets;
    int presetCount;

    uint32_t tick;

    VoicePool *voicePool;
    Voice *voiceFirst;
    Voice *voiceLast;

    double sampleRate;
    double gain;
    double masterVolume;

    Channel channels[CHANNEL_COUNT];

    Callback *callbackList;
    int32_t callbackListLength;

    struct {
        bool enable;
        Level master;
        Level channels[CHANNEL_COUNT];
    } level;
};

static Preset *SynthesizerFindPreset(Synthesizer *self, uint16_t midiPresetNo, uint16_t bankNo);
static int PresetComparator(const void *preset1, const void *preset2);
static int SynthesizerNoteOn(Synthesizer *self, uint8_t channel, uint8_t noteNo, uint8_t velocity);
static void SynthesizerNoteOff(Synthesizer *self, uint8_t channel, uint8_t noteNo);
static void SynthesizerReleaseExclusiveClass(Synthesizer *self, Voice *newVoice);
static void SynthesizerReleaseIdenticalVoice(Synthesizer *self, Voice *newVoice);
static void SynthesizerProgramChange(Synthesizer *self, uint8_t channel, uint8_t programNo);
static void SynthesizerControlChange(Synthesizer *self, uint8_t channel, uint8_t ccNumber, uint8_t value);
static void SynthesizerAddVoice(Synthesizer *self, Voice *voice);
static void SynthesizerRemoveVoice(Synthesizer *self, Voice *voice);
static void SynthesizerComputeAudioSample(Synthesizer *self, AudioSample *buffer, double *chorusSend, double *reverbSend, uint32_t count);
static void SynthesizerNotifyEvent(Synthesizer *self, MidiSourceEvent event, void *arg1, void *arg2);

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

static void registerCallback(void *_self, MidiSourceCallback function, void *receiver)
{
    Synthesizer *self = _self;

    self->callbackList = realloc(self->callbackList, sizeof(Callback) * (self->callbackListLength + 1));
    self->callbackList[self->callbackListLength].function = function; 
    self->callbackList[self->callbackListLength].receiver = receiver; 
    ++self->callbackListLength;
}

static void unregisterCallback(void *_self, MidiSourceCallback function, void *receiver)
{
    Synthesizer *self = _self;

    for (int i = 0; i < self->callbackListLength; ++i) {
        if (self->callbackList[i].function == function
                && self->callbackList[i].receiver == receiver) {

            size_t moveLength = self->callbackListLength - 1 - i;
            if (0 < moveLength) {
                memmove(&self->callbackList[i], &self->callbackList[i + 1], sizeof(Callback) * moveLength);
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

static void destroy(void *self)
{
    SynthesizerDestroy(self);
}

static int getPresetCount(void *self)
{
    return ((Synthesizer *)self)->presetCount;
}

static PresetInfo **getPresetInfos(void *_self)
{
    Synthesizer *self = _self;
    return self->presetInfos;
}

static PresetInfo *getPresetInfo(void *_self, uint8_t channel)
{
    Synthesizer *self = _self;
    Preset **result = bsearch(&self->channels[channel].preset, self->presets, self->presetCount, sizeof(Preset *), PresetComparator);
    int index = (int)(result - self->presets);
    return self->presetInfos[index];
}

static void setLevelEnable(void *self, bool enable)
{
    ((Synthesizer *)self)->level.enable = enable;
}

static Level getMasterLevel(void *self)
{
    return ((Synthesizer *)self)->level.master;
}

static Level getChannelLevel(void *self, uint8_t channel)
{
    return ((Synthesizer *)self)->level.channels[channel];
}

static void setPresetInfo(void *_self, uint8_t channel, PresetInfo *presetInfo)
{
    Synthesizer *self = _self;
    SynthesizerControlChange(self, channel, CC_BankSelect_MSB, 0x007F & (presetInfo->bankNo >> 7));
    SynthesizerControlChange(self, channel, CC_BankSelect_LSB, 0x007F & presetInfo->bankNo);
    SynthesizerProgramChange(self, channel, presetInfo->programNo);
}

static void setGain(void *_self, int16_t cb)
{
    Synthesizer *self = _self;
    self->gain = cB2Value(cb);
    SynthesizerNotifyEvent(self, MidiSourceEventChangeGain, &self->gain, NULL);
}

static void setMasterVolume(void *_self, int16_t cb)
{
    Synthesizer *self = _self;
    self->masterVolume = cB2Value(cb);
    SynthesizerNotifyEvent(self, MidiSourceEventChangeMasterVolume, &self->masterVolume, NULL);
}

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

static void setExpressionSend(void *self, uint8_t channel, uint8_t value)
{
    SynthesizerControlChange(self, channel, CC_Expression_MSB, value);
}

static int16_t getGain(void *self)
{
    return Value2cB(((Synthesizer *)self)->gain);
}

static int16_t getMasterVolume(void *self)
{
    return Value2cB(((Synthesizer *)self)->masterVolume);
}

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

static uint8_t getExpressionSend(void *self, uint8_t channel)
{
    return ((Synthesizer *)self)->channels[channel].cc[CC_Expression_MSB];
}

static void computeAudioSample(void *self, AudioSample *buffer, double *chorusSend, double *reverbSend, uint32_t count)
{
    SynthesizerComputeAudioSample((Synthesizer *)self, buffer, chorusSend, reverbSend, count);
}


Synthesizer *SynthesizerCreate(SoundFont *sf, double sampleRate)
{
    Synthesizer *self = calloc(1, sizeof(Synthesizer));

    self->srcVtbl.send = send;
    self->srcVtbl.isAvailable = isAvailable;
    self->srcVtbl.computeAudioSample = computeAudioSample;
    self->srcVtbl.registerCallback = registerCallback;
    self->srcVtbl.unregisterCallback = unregisterCallback;
    self->srcVtbl.destroy = destroy;
    self->srcVtbl.getName = getName;
    self->srcVtbl.getPresetCount = getPresetCount;
    self->srcVtbl.getPresetInfos = getPresetInfos;
    self->srcVtbl.getPresetInfo = getPresetInfo;
    self->srcVtbl.setPresetInfo = setPresetInfo;
    self->srcVtbl.setLevelEnable = setLevelEnable;
    self->srcVtbl.getMasterLevel = getMasterLevel;
    self->srcVtbl.getChannelLevel = getChannelLevel;
    self->srcVtbl.setGain = setGain;
    self->srcVtbl.setMasterVolume = setMasterVolume;
    self->srcVtbl.setVolume = setVolume;
    self->srcVtbl.setPan = setPan;
    self->srcVtbl.setChorusSend = setChorusSend;
    self->srcVtbl.setReverbSend = setReverbSend;
    self->srcVtbl.setExpressionSend = setExpressionSend;
    self->srcVtbl.getGain = getGain;
    self->srcVtbl.getMasterVolume = getMasterVolume;
    self->srcVtbl.getVolume = getVolume;
    self->srcVtbl.getPan = getPan;
    self->srcVtbl.getChorusSend = getChorusSend;
    self->srcVtbl.getReverbSend = getReverbSend;
    self->srcVtbl.getExpressionSend = getExpressionSend;

    self->sf = sf;

    self->sampleRate = sampleRate;

    self->voicePool = VoicePoolCreate();

    self->gain = cB2Value(-100);
    self->masterVolume = cB2Value(0);

    ParsePresets(self->sf, &self->presets, &self->presetCount);
    qsort(self->presets, self->presetCount, sizeof(Preset *), PresetComparator);

    self->presetInfos = calloc(self->presetCount, sizeof(PresetInfo *));

    for (int i = 0; i < self->presetCount; ++i) {
        self->presetInfos[i] = calloc(1, sizeof(PresetInfo));
        self->presetInfos[i]->name = self->presets[i]->name;
        self->presetInfos[i]->bankNo = self->presets[i]->bankNo;
        self->presetInfos[i]->programNo = self->presets[i]->midiPresetNo;
    }

    if (0 < self->presetCount) {
        for (int i = 0; i < CHANNEL_COUNT; ++i) {
            ChannelInitialize(&self->channels[i], i, self->presets[0]);
        }
    }

    self->level.enable = true;

    return self;
}

void SynthesizerDestroy(Synthesizer *self)
{
    for (int i = 0; i < self->presetCount; ++i) {
        PresetDestroy(self->presets[i]);
    }

    for (int i = 0; i < self->presetCount; ++i) {
        free(self->presetInfos[i]);
    }
    free(self->presetInfos);

    if (self->presets) {
        free(self->presets);
    }

    if (self->callbackList) {
        free(self->callbackList);
    }

    VoicePoolDestroy(self->voicePool);

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
                    self->sf, self->sampleRate, self->gain, self->masterVolume);

            SynthesizerReleaseExclusiveClass(self, voice);
            SynthesizerReleaseIdenticalVoice(self, voice);

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

static void SynthesizerReleaseIdenticalVoice(Synthesizer *self, Voice *newVoice)
{
    for (Voice *voice = self->voiceFirst; NULL != voice; voice = voice->next) {
        if (voice->channel == newVoice->channel && voice->key == newVoice->key
                && voice->instrumentZone->sample == newVoice->instrumentZone->sample) {
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

    switch (ccNumber) {
    case CC_Volume_MSB:
        SynthesizerNotifyEvent(self, MidiSourceEventChangeVolume, &channel, &value);
        break;
    case CC_Pan_MSB:
        SynthesizerNotifyEvent(self, MidiSourceEventChangePan, &channel, &value);
        break;
    case CC_Effect3Depth:
        SynthesizerNotifyEvent(self, MidiSourceEventChangeChorusSend, &channel, &value);
        break;
    case CC_Effect1Depth:
        SynthesizerNotifyEvent(self, MidiSourceEventChangeReverbSend, &channel, &value);
        break;
    case CC_Expression_MSB:
        SynthesizerNotifyEvent(self, MidiSourceEventChangeExpressionSend, &channel, &value);
        break;
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
        PresetInfo *presetInfo = getPresetInfo(self, channel);
        SynthesizerNotifyEvent(self, MidiSourceEventChangePreset, &channel, presetInfo);
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
}

typedef struct _LevelMater {
    AudioSample master;
    AudioSample channels[CHANNEL_COUNT];
    struct {
        AudioSample channels[CHANNEL_COUNT];
    } buffer;
} LevelMater;

static void LevelMaterAddToChannel(LevelMater *self, uint8_t channel, AudioSample *sample)
{
    self->buffer.channels[channel].L += sample->L;
    self->buffer.channels[channel].R += sample->R;
}

static void LevelMaterUpdate(LevelMater *self, AudioSample *master)
{
    self->master.L = MAX(self->master.L, fabs(master->L));
    self->master.R = MAX(self->master.R, fabs(master->R));

    for (int i = 0; i < CHANNEL_COUNT; ++i) {
        self->channels[i].L = MAX(self->channels[i].L, fabs(self->buffer.channels[i].L));
        self->channels[i].R = MAX(self->channels[i].R, fabs(self->buffer.channels[i].R));

        self->buffer.channels[i].L = 0.0f;
        self->buffer.channels[i].R = 0.0f;
    }
}

static void LevelMaterNormalize(LevelMater *self, Level *master, Level *channels)
{
    master->L = Value2cB(self->master.L);
    master->R = Value2cB(self->master.R);

    for (int i = 0; i < CHANNEL_COUNT; ++i) {
        channels[i].L = Value2cB(self->channels[i].L);
        channels[i].R = Value2cB(self->channels[i].R);
    }
}

static void SynthesizerComputeAudioSample(Synthesizer *self, AudioSample *buffer, double *chorusSend, double *reverbSend, uint32_t count)
{
    LevelMater levelMater = {};

    for (int i = 0; i < count; ++i) {
        AudioSample direct = { .L = 0.0, .R = 0.0 };
        double chorus = 0.0;
        double reverb = 0.0;

        bool needUpdate = 0 == i % UPDATE_THRESHOLD;

        for (Voice *voice = self->voiceFirst; NULL != voice;) {
            if (needUpdate) {
                VoiceUpdate(voice);

                if (VoiceIsReleased(voice)) {
                    Voice *toFree = voice;
                    voice = voice->next;
                    SynthesizerRemoveVoice(self, toFree);
                    VoicePoolDeallocVoice(self->voicePool, toFree);
                    continue;
                }
            }

            double computed = VoiceComputeSample(voice);
            
            AudioSample sample;
            sample.L = computed * voice->computed.leftAmplifier;
            sample.R = computed * voice->computed.rightAmplifier;

            if (needUpdate && self->level.enable) {
                LevelMaterAddToChannel(&levelMater, voice->channel->number, &sample);
            }

            direct.L += sample.L;
            direct.R += sample.R;

            chorus += computed * voice->computed.chorusEffectsSend;
            reverb += computed * voice->computed.reverbEffectsSend;

            VoiceIncrementSample(voice);

            voice = voice->next;
        }

        if (needUpdate && self->level.enable) {
            LevelMaterUpdate(&levelMater, &direct);
        }

        buffer[i].L += direct.L;
        buffer[i].R += direct.R;

        chorusSend[i] += chorus;
        reverbSend[i] += reverb;
    }

    if (self->level.enable) {
        LevelMaterNormalize(&levelMater, &self->level.master, self->level.channels);
        SynthesizerNotifyEvent(self, MidiSourceEventChangeLevelMater, &self->level.master, self->level.channels);
    }

    self->tick += count;
}

static void SynthesizerNotifyEvent(Synthesizer *self, MidiSourceEvent event, void *arg1, void *arg2)
{
    for (int i = self->callbackListLength - 1; 0 <= i; --i) {
        self->callbackList[i].function(self->callbackList[i].receiver, (MidiSource *)self, event, arg1, arg2);
    }
}
