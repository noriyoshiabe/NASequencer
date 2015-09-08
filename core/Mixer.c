#include "Mixer.h"
#include "Synthesizer.h"
#include "AudioOut.h"
#include "Define.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

struct _Mixer {
    NAArray *observers;
    NAArray *channels;
    Level level;
};

struct _MixerChannel {
    int number;
    MidiSource *source;
    bool mute;
    bool solo;
    bool active;
    Mixer *mixer;
};

typedef struct Observer {
    void *receiver;
    MixerObserverCallbacks *callbacks;
} Observer;

static void MixerAudioCallback(void *receiver, AudioSample *buffer, uint32_t count);

Mixer *MixerCreate()
{
    Mixer *self =  calloc(1, sizeof(Mixer));

    self->observers = NAArrayCreate(4, NULL);

    // TODO move to midisource manager
    char filepath[PATH_MAX];
    sprintf(filepath, "%s/.namidi/GeneralUser GS Live-Audigy v1.44.sf2", getenv("HOME"));
    SoundFontError error;
    SoundFont *soundFont = SoundFontRead(filepath, &error);
    AudioOut *audioOut = AudioOutSharedInstance();
    MidiSource *source = (MidiSource *)SynthesizerCreate(soundFont, AudioOutGetSampleRate(audioOut));

    self->channels = NAArrayCreate(16, NULL);
    for (int i = 0; i < 16; ++i) {
        MixerChannel *channel = calloc(1, sizeof(MixerChannel));
        channel->number = i + 1;
        channel->source = source;
        channel->active = true;
        channel->mixer = self;
        NAArrayAppend(self->channels, channel);
    }

    AudioOutRegisterCallback(AudioOutSharedInstance(), MixerAudioCallback, self);

    return self;
}

void MixerDestroy(Mixer *self)
{
    NAArrayTraverse(self->observers, free);
    NAArrayDestroy(self->observers);
    NAArrayTraverse(self->channels, free);
    NAArrayDestroy(self->channels);
    free(self);
}

void MixerAddObserver(Mixer *self, void *receiver, MixerObserverCallbacks *callbacks)
{
    Observer *observer = malloc(sizeof(Observer));
    observer->receiver = receiver;
    observer->callbacks = callbacks;
    NAArrayAppend(self->observers, observer);
}

static int MixerObserverFindComparator(const void *receiver, const void *observer)
{
    return receiver - ((Observer *)observer)->receiver;
}

void MixerRemoveObserver(Mixer *self, void *receiver)
{
    int index = NAArrayFindFirstIndex(self->observers, receiver, MixerObserverFindComparator);
    NAArrayApplyAt(self->observers, index, free);
    NAArrayRemoveAt(self->observers, index);
}

void MixerSendNoteOn(Mixer *self, NoteEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        uint8_t bytes[3] = {0x90 | (0x0F & channel->number), event->noteNo, event->velocity};
        channel->source->send(channel->source, bytes, 3);
    }
}

void MixerSendNoteOff(Mixer *self, NoteEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        uint8_t bytes[3] = {0x80 | (0x0F & channel->number), event->noteNo, 0};
        channel->source->send(channel->source, bytes, 3);
    }
}

void MixerSendAllNoteOff(Mixer *self)
{
    uint8_t bytes[3] = {0, 0x7B, 0x00};
    for (int i = 0; i < 16; ++i) {
        MixerChannel *channel = NAArrayGetValueAt(self->channels, i);
        bytes[0] = 0xB0 | i;
        channel->source->send(channel->source, bytes, 3);
    }
}

void MixerSendVoice(Mixer *self, VoiceEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        uint8_t bytes[3];

        bytes[0] = 0xB0 | (0x0F & channel->number);
        bytes[1] = 0x00;
        bytes[2] = event->msb;
        channel->source->send(channel->source, bytes, 3);

        bytes[1] = 0x20;
        bytes[2] = event->lsb;
        channel->source->send(channel->source, bytes, 3);

        bytes[0] = 0xC0 | (0x0F & channel->number);
        bytes[1] = event->programNo;
        channel->source->send(channel->source, bytes, 2);
    }
}

void MixerSendVolume(Mixer *self, VolumeEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        channel->source->setVolume(channel->source, channel->number, event->value);
    }
}

void MixerSendPan(Mixer *self, PanEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        channel->source->setPan(channel->source, channel->number, event->value);
    }
}

void MixerSendChorus(Mixer *self, ChorusEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        channel->source->setChorusSend(channel->source, channel->number, event->value);
    }
}

void MixerSendReverb(Mixer *self, ReverbEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        channel->source->setReverbSend(channel->source, channel->number, event->value);
    }
}

NAArray *MixerGetChannels(Mixer *self)
{
    return self->channels;
}

static void MixerAudioCallback(void *receiver, AudioSample *buffer, uint32_t count)
{
    Mixer *self = receiver;

    AudioSample samples[count];
    AudioSample *p = samples;

    for (int i = 0; i < count; ++i) {
        *p++ = (AudioSample){0, 0};
    }

    // TODO manage multiple midi sources
    MixerChannel *channel = NAArrayGetValueAt(self->channels, 0);
    channel->source->computeAudioSample(channel->source, samples, count);

    AudioSample valueLevel = {0, 0};

    for (int i = 0; i < count; ++i) {
        buffer[i].L += samples[i].L;
        buffer[i].R += samples[i].R;

        valueLevel.L = MAX(valueLevel.L, fabs(samples[i].L));
        valueLevel.R = MAX(valueLevel.L, fabs(samples[i].R));
    }

    self->level.L = Value2cB(valueLevel.L);
    self->level.R = Value2cB(valueLevel.R);
}

static void MixerUpdateActiveChannles(Mixer *self)
{
    bool soloExists = false;

    int count = NAArrayCount(self->channels);
    MixerChannel **channels = NAArrayGetValues(self->channels);
    for (int i = 0; i < count; ++i) {
        if (channels[i]->solo) {
            soloExists = true;
            break;
        }
    }

    for (int i = 0; i < count; ++i) {
        MixerChannel *channel = channels[i];
        channel->active = !channel->mute && (channel->solo || !soloExists);
    }
}


int MixerChannelGetNumber(MixerChannel *self)
{
    return self->number;
}

int MixerChannelGetPresetCount(MixerChannel *self)
{
    return self->source->getPresetCount(self->source);
}

PresetInfo **MixerChannelGetPresetInfos(MixerChannel *self)
{
    return self->source->getPresetInfos(self->source);
}

PresetInfo *MixerChannelGetPresetInfo(MixerChannel *self)
{
    return self->source->getPresetInfo(self->source, self->number);
}

Level MixerChannelGetLevel(MixerChannel *self)
{
    return self->source->getChannelLevel(self->source, self->number);
}

int MixerChannelGetVolume(MixerChannel *self)
{
    return self->source->getVolume(self->source, self->number);
}

int MixerChannelGetPan(MixerChannel *self)
{
    return self->source->getPan(self->source, self->number);
}

int MixerChannelGetChorusSend(MixerChannel *self)
{
    return self->source->getChorusSend(self->source, self->number);
}

int MixerChannelGetReverbSend(MixerChannel *self)
{
    return self->source->getReverbSend(self->source, self->number);
}

bool MixerChannelGetMute(MixerChannel *self)
{
    return self->mute;
}

bool MixerChannelGetSolo(MixerChannel *self)
{
    return self->solo;
}

void MixerChannelSetPresetInfo(MixerChannel *self, PresetInfo *presetInfo)
{
    self->source->setPresetInfo(self->source, self->number, presetInfo);
}

void MixerChannelSetVolume(MixerChannel *self, int value)
{
    self->source->setVolume(self->source, self->number, value);
}

void MixerChannelSetPan(MixerChannel *self, int value)
{
    self->source->setPan(self->source, self->number, value);
}

void MixerChannelSetChorusSend(MixerChannel *self, int value)
{
    self->source->setChorusSend(self->source, self->number, value);
}

void MixerChannelSetReverbSend(MixerChannel *self, int value)
{
    self->source->setReverbSend(self->source, self->number, value);
}

void MixerChannelSetMute(MixerChannel *self, bool mute)
{
    self->mute = mute;
    MixerUpdateActiveChannles(self->mixer);
}

void MixerChannelSetSolo(MixerChannel *self, bool solo)
{
    self->solo = solo;
    MixerUpdateActiveChannles(self->mixer);
}
