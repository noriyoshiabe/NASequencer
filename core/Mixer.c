#include "Mixer.h"
#include "Define.h"
#include "NAMap.h"
#include "NAMessageQ.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef enum _MixerMessage {
    MixerMessageAttachSource,
    MixerMessageDetachSource,
    MixerMessageDestroy,
} MixerMessage;

struct _Mixer {
    AudioOut *audioOut;
    NAArray *observers;
    NAArray *channels;
    Level level;
    NAMap *sourceMap;

    NAMessageQ *msgQ;
    NAArray *activeSources;
};

struct _MixerChannel {
    int number;
    MidiSourceDescription *description;
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

static MidiSourceManagerObserverCallbacks MixerMidiSourceManagerObserverCallbacks;

static void MixerAudioCallback(void *receiver, AudioSample *buffer, uint32_t count);
static void MixerMidiSourceCallback(void *receiver, MidiSource *source, MidiSourceEvent event, void *arg1, void *arg2);
static void MixerProcessMessage(Mixer *self);

Mixer *MixerCreate(AudioOut *audioOut)
{
    Mixer *self = calloc(1, sizeof(Mixer));

    self->audioOut = audioOut;
    self->observers = NAArrayCreate(4, NULL);

    MidiSourceManager *manager = MidiSourceManagerSharedInstance();
    MidiSourceManagerAddObserver(manager, self, &MixerMidiSourceManagerObserverCallbacks);

    MidiSourceDescription *description = MidiSourceManagerGetDefaultDescription(manager);
    MidiSource *source = MidiSourceManagerAllocMidiSource(manager, description, self->audioOut->getSampleRate(self->audioOut));

    self->sourceMap = NAMapCreate(NAHashAddress, NULL, NULL);
    NAMapPut(self->sourceMap, description, source);

    self->msgQ = NAMessageQCreate(32);
    self->activeSources = NAArrayCreate(2, NULL);
    NAArrayAppend(self->activeSources, source);

    self->channels = NAArrayCreate(16, NULL);
    for (int i = 0; i < 16; ++i) {
        MixerChannel *channel = calloc(1, sizeof(MixerChannel));
        channel->number = i + 1;
        channel->description = description;
        channel->source = source;
        channel->active = true;
        channel->mixer = self;
        NAArrayAppend(self->channels, channel);
    }

    self->audioOut->registerCallback(self->audioOut, MixerAudioCallback, self);

    return self;
}

void MixerDestroy(Mixer *self)
{
    MidiSourceManagerRemoveObserver(MidiSourceManagerSharedInstance(), self);
    NAMessageQPost(self->msgQ, MixerMessageDestroy, NULL);
}

static void _UnregisterCallback(Mixer *self, MidiSource *source, va_list argList)
{
    source->unregisterCallback(source, MixerMidiSourceCallback, self);
}

static void _DeallocMidiSource(MidiSource *source)
{
    MidiSourceManagerDeallocMidiSource(MidiSourceManagerSharedInstance(), source);
}

static void _MixerDestroy(Mixer *self)
{
    self->audioOut->unregisterCallback(self->audioOut, MixerAudioCallback, self);

    NAMessageQDestroy(self->msgQ);

    NAArrayTraverseWithContext(self->activeSources, self, _UnregisterCallback, NULL);
    NAArrayDestroy(self->activeSources);

    NAMapTraverseValue(self->sourceMap, _DeallocMidiSource);
    NAMapDestroy(self->sourceMap);

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
        int value = Clip(event->value + 64, 0, 127);
        channel->source->setPan(channel->source, channel->number, value);
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

void MixerSendExpression(Mixer *self, ExpressionEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        channel->source->setExpressionSend(channel->source, channel->number, event->value);
    }
}

void MixerSendDetune(Mixer *self, DetuneEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        uint8_t bytes[3];

        bytes[0] = 0xB0 | (0x0F & channel->number);

        bytes[1] = 101;
        bytes[2] = 0;
        channel->source->send(channel->source, bytes, 3);

        bytes[1] = 100;
        bytes[2] = 1;
        channel->source->send(channel->source, bytes, 3);

        bytes[1] = 6;
        bytes[2] = event->fine.msb;
        channel->source->send(channel->source, bytes, 3);

        bytes[1] = 38;
        bytes[2] = event->fine.lsb;
        channel->source->send(channel->source, bytes, 3);

        bytes[1] = 101;
        bytes[2] = 0;
        channel->source->send(channel->source, bytes, 3);

        bytes[1] = 100;
        bytes[2] = 2;
        channel->source->send(channel->source, bytes, 3);

        bytes[1] = 6;
        bytes[2] = event->corse.msb;
        channel->source->send(channel->source, bytes, 3);

        bytes[1] = 101;
        bytes[2] = 0x7F;
        channel->source->send(channel->source, bytes, 3);
        bytes[1] = 100;
        channel->source->send(channel->source, bytes, 3);
    }
}

void MixerSendSynth(Mixer *self, SynthEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    NAArray *availableDescriptions = MidiSourceManagerGetAvailableDescriptions(MidiSourceManagerSharedInstance());
    NAIterator *iterator = NAArrayGetIterator(availableDescriptions);
    while (iterator->hasNext(iterator)) {
        MidiSourceDescription *description = iterator->next(iterator);
        if (0 == strcmp(description->name, event->identifier)) {
            MixerChannelSetMidiSourceDescription(channel, description);
            break;
        }
    }
}

NAArray *MixerGetChannels(Mixer *self)
{
    return self->channels;
}

static void MixerProcessMessage(Mixer *self)
{
    NAMessage msg;

    while (NAMessageQPeek(self->msgQ, &msg)) {
        switch (msg.kind) {
        case MixerMessageAttachSource:
            if (-1 == NAArrayFindFirstIndex(self->activeSources, msg.data, NAArrayAddressComparator)) {
                NAArrayAppend(self->activeSources, msg.data);

                MidiSource *source = msg.data;
                source->registerCallback(source, MixerMidiSourceCallback, self);
            }
            break;
        case MixerMessageDetachSource:
            {
                MidiSource *source = msg.data;

                int index = NAArrayFindFirstIndex(self->activeSources, source, NAArrayAddressComparator);
                if (-1 != index) {
                    NAArrayRemoveAt(self->activeSources, index);

                    source->unregisterCallback(source, MixerMidiSourceCallback, self);
                }

                MidiSourceManagerDeallocMidiSource(MidiSourceManagerSharedInstance(), source);
            }
            break;
        case MixerMessageDestroy:
            _MixerDestroy(self);
            return;
        }
    }
}

static void MixerAudioCallback(void *receiver, AudioSample *buffer, uint32_t count)
{
    Mixer *self = receiver;

    AudioSample samples[count];
    AudioSample *p = samples;

    for (int i = 0; i < count; ++i) {
        *p++ = (AudioSample){0, 0};
    }

    NAIterator *iterator = NAArrayGetIterator(self->activeSources);
    while (iterator->hasNext(iterator)) {
        MidiSource *source = iterator->next(iterator);
        source->computeAudioSample(source, samples, count);
    }

    AudioSample valueLevel = {0, 0};

    for (int i = 0; i < count; ++i) {
        buffer[i].L += samples[i].L;
        buffer[i].R += samples[i].R;

        valueLevel.L = MAX(valueLevel.L, fabs(samples[i].L));
        valueLevel.R = MAX(valueLevel.R, fabs(samples[i].R));
    }

    self->level.L = Value2cB(valueLevel.L);
    self->level.R = Value2cB(valueLevel.R);

    MixerProcessMessage(self);
}

static void _MixerNotifyChannelStatusChange(Mixer *self, Observer *observer, va_list argList)
{
    observer->callbacks->onChannelStatusChange(observer->receiver, va_arg(argList, MixerChannel *));
}

static void MixerNotifyChannelStatusChange(Mixer *self, MixerChannel *channel)
{
    NAArrayTraverseWithContext(self->observers, self, _MixerNotifyChannelStatusChange, channel);
}

static void _MixerNotifyAvailableMidiSourceChange(Mixer *self, Observer *observer, va_list argList)
{
    observer->callbacks->onAvailableMidiSourceChange(observer->receiver, va_arg(argList, NAArray *));
}

static void MixerNotifyAvailableMidiSourceChange(Mixer *self, NAArray *descriptions)
{
    NAArrayTraverseWithContext(self->observers, self, _MixerNotifyAvailableMidiSourceChange, descriptions);
}

static void _MixerNotifyLevelUpdate(Mixer *self, Observer *observer, va_list argList)
{
    observer->callbacks->onLevelUpdate(observer->receiver);
}

static void MixerNotifyLevelUpdate(Mixer *self)
{
    NAArrayTraverseWithContext(self->observers, self, _MixerNotifyLevelUpdate, NULL);
}

static void MixerMidiSourceCallback(void *receiver, MidiSource *source, MidiSourceEvent event, void *arg1, void *arg2)
{
    Mixer *self = receiver;

    switch (event) {
    case MidiSourceEventChangeMasterGain:
    case MidiSourceEventChangeMasterVolume:
        break;
    case MidiSourceEventChangeVolume:
    case MidiSourceEventChangePan:
    case MidiSourceEventChangeChorusSend:
    case MidiSourceEventChangeReverbSend:
    case MidiSourceEventChangeExpressionSend:
    case MidiSourceEventChangePreset:
        {
            uint8_t channel = *((uint8_t *)arg1);
            MixerNotifyChannelStatusChange(self, NAArrayGetValueAt(self->channels, channel));
        }
        break;
    case MidiSourceEventChangeLevelMater:
        {
            MixerNotifyLevelUpdate(self);
        }
        break;
    }
}

static void MixerUpdateActiveChannles(Mixer *self)
{
    NAIterator *iterator;
    bool soloExists = false;

    iterator = NAArrayGetIterator(self->channels);
    while (iterator->hasNext(iterator)) {
        MixerChannel *channel = iterator->next(iterator);
        if (channel->solo) {
            soloExists = true;
            break;
        }
    }

    iterator = NAArrayGetIterator(self->channels);
    while (iterator->hasNext(iterator)) {
        MixerChannel *channel = iterator->next(iterator);
        bool active = !channel->mute && (channel->solo || !soloExists);
        bool notify = channel->active != active;

        channel->active = active;

        if (notify) {
            MixerNotifyChannelStatusChange(self, channel);
        }
    }
}

static void MixerMidiSourceManagerOnLoadMidiSourceDescription(void *receiver, MidiSourceDescription *description)
{
}

static void MixerMidiSourceManagerOnLoadAvailableMidiSourceDescription(void *receiver, MidiSourceDescription *description)
{
    MidiSourceManager *manager = MidiSourceManagerSharedInstance();
    MixerNotifyAvailableMidiSourceChange(receiver, MidiSourceManagerGetAvailableDescriptions(manager));
}

static void MixerMidiSourceManagerOnUnloadMidiSourceDescription(void *receiver, MidiSourceDescription *description)
{
}

static void MixerMidiSourceManagerOnUnloadAvailableMidiSourceDescription(void *receiver, MidiSourceDescription *description)
{
    Mixer *self = receiver;

    MidiSourceManager *manager = MidiSourceManagerSharedInstance();
    NAArray *descriptions = MidiSourceManagerGetAvailableDescriptions(manager);

    MidiSource *sourceToUnload = NAMapRemove(self->sourceMap, description);
    if (sourceToUnload) {
        MidiSourceDescription *defaultDescription = MidiSourceManagerGetDefaultDescription(manager);

        NAIterator *iterator = NAArrayGetIterator(self->channels);
        while (iterator->hasNext(iterator)) {
            MixerChannel *channel = iterator->next(iterator);
            if (channel->source == sourceToUnload) {
                MidiSource *source = NAMapGet(self->sourceMap, defaultDescription); 
                if (!source) {
                    source = MidiSourceManagerAllocMidiSource(manager, defaultDescription, self->audioOut->getSampleRate(self->audioOut));
                    NAMapPut(self->sourceMap, defaultDescription, source);
                }

                channel->source = source;
                channel->description = defaultDescription;

                NAMessageQPost(self->msgQ, MixerMessageAttachSource, source);
            }
        }

        NAMessageQPost(self->msgQ, MixerMessageDetachSource, sourceToUnload);
    }

    MixerNotifyAvailableMidiSourceChange(receiver, descriptions);
}

static MidiSourceManagerObserverCallbacks MixerMidiSourceManagerObserverCallbacks = {
    MixerMidiSourceManagerOnLoadMidiSourceDescription,
    MixerMidiSourceManagerOnLoadAvailableMidiSourceDescription,
    MixerMidiSourceManagerOnUnloadMidiSourceDescription,
    MixerMidiSourceManagerOnUnloadAvailableMidiSourceDescription
};


int MixerChannelGetNumber(MixerChannel *self)
{
    return self->number;
}

MidiSourceDescription *MixerChannelGetMidiSourceDescription(MixerChannel *self)
{
    return self->description;
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
    return self->source->getPresetInfo(self->source, self->number - 1);
}

Level MixerChannelGetLevel(MixerChannel *self)
{
    return self->source->getChannelLevel(self->source, self->number - 1);
}

int MixerChannelGetVolume(MixerChannel *self)
{
    return self->source->getVolume(self->source, self->number - 1);
}

int MixerChannelGetPan(MixerChannel *self)
{
    return self->source->getPan(self->source, self->number - 1);
}

int MixerChannelGetChorusSend(MixerChannel *self)
{
    return self->source->getChorusSend(self->source, self->number - 1);
}

int MixerChannelGetReverbSend(MixerChannel *self)
{
    return self->source->getReverbSend(self->source, self->number - 1);
}

int MixerChannelGetExpressionSend(MixerChannel *self)
{
    return self->source->getExpressionSend(self->source, self->number - 1);
}

bool MixerChannelGetMute(MixerChannel *self)
{
    return self->mute;
}

bool MixerChannelGetSolo(MixerChannel *self)
{
    return self->solo;
}

void MixerChannelSetMidiSourceDescription(MixerChannel *self, MidiSourceDescription *description)
{
    self->source = NAMapGet(self->mixer->sourceMap, description);

    if (!self->source) {
        MidiSourceManager *manager = MidiSourceManagerSharedInstance();
        AudioOut *audioOut = self->mixer->audioOut;
        self->source = MidiSourceManagerAllocMidiSource(manager, description, audioOut->getSampleRate(audioOut));
        NAMapPut(self->mixer->sourceMap, description, self->source);
    }

    NAMessageQPost(self->mixer->msgQ, MixerMessageAttachSource, self->source);

    MixerNotifyChannelStatusChange(self->mixer, self);
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

void MixerChannelSetExpressionSend(MixerChannel *self, int value)
{
    self->source->setExpressionSend(self->source, self->number, value);
}

void MixerChannelSetMute(MixerChannel *self, bool mute)
{
    self->mute = mute;
    MixerNotifyChannelStatusChange(self->mixer, self);
    MixerUpdateActiveChannles(self->mixer);
}

void MixerChannelSetSolo(MixerChannel *self, bool solo)
{
    self->solo = solo;
    MixerNotifyChannelStatusChange(self->mixer, self);
    MixerUpdateActiveChannles(self->mixer);
}
