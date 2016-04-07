#include "Mixer.h"
#include "Define.h"
#include "Chorus.h"
#include "Reverb.h"
#include "Voice.h"
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
    bool levelEnable;
    Level level;
    NAMap *sourceMap;

    NAMessageQ *msgQ;
    NAArray *activeSources;

    double sampleRate;
    Chorus *chorus;
    Reverb *reverb;
};

struct _MixerChannel {
    int number;
    int midiNumber;
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
    
    self->sampleRate = self->audioOut->getSampleRate(self->audioOut);

    MidiSourceManager *manager = MidiSourceManagerSharedInstance();
    MidiSourceManagerAddObserver(manager, self, &MixerMidiSourceManagerObserverCallbacks);

    MidiSourceDescription *description = MidiSourceManagerGetDefaultDescription(manager);
    MidiSource *source = MidiSourceManagerAllocMidiSource(manager, description, self->sampleRate);

    self->sourceMap = NAMapCreate(NAHashAddress, NULL, NULL);
    NAMapPut(self->sourceMap, description, source);

    self->msgQ = NAMessageQCreate(32);
    self->activeSources = NAArrayCreate(2, NULL);
    NAArrayAppend(self->activeSources, source);

    self->channels = NAArrayCreate(16, NULL);
    for (int i = 0; i < 16; ++i) {
        MixerChannel *channel = calloc(1, sizeof(MixerChannel));
        channel->number = i + 1;
        channel->midiNumber = i;
        channel->description = description;
        channel->source = source;
        channel->active = true;
        channel->mixer = self;
        NAArrayAppend(self->channels, channel);
    }

    self->chorus = ChorusCreate(self->sampleRate);

    ChorusAddDelay(self->chorus, 0.020, 1.00, 0.001, 0.7, 0.7);
    ChorusAddDelay(self->chorus, 0.020, 0.50, 0.001, 1.0, 0.0);
    ChorusAddDelay(self->chorus, 0.020, 0.25, 0.001, 0.0, 1.0);

    self->reverb = ReverbCreate(self->sampleRate, 1.0);

    self->audioOut->registerCallback(self->audioOut, MixerAudioCallback, self);

    self->levelEnable = true;
    source->setLevelEnable(source, self->levelEnable);
    source->registerCallback(source, MixerMidiSourceCallback, self);

    return self;
}

void MixerDestroy(Mixer *self)
{
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

    MidiSourceManagerRemoveObserver(MidiSourceManagerSharedInstance(), self);

    NAMapTraverseValue(self->sourceMap, _DeallocMidiSource);
    NAMapDestroy(self->sourceMap);

    NAArrayTraverse(self->observers, free);
    NAArrayDestroy(self->observers);
    NAArrayTraverse(self->channels, free);
    NAArrayDestroy(self->channels);

    ChorusDestroy(self->chorus);
    ReverbDestroy(self->reverb);
    
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
        uint8_t bytes[3] = {0x90 | (0x0F & channel->midiNumber), event->noteNo, event->velocity};
        channel->source->send(channel->source, bytes, 3);
    }
}

void MixerSendNoteOff(Mixer *self, NoteEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        uint8_t bytes[3] = {0x80 | (0x0F & channel->midiNumber), event->noteNo, 0};
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

void MixerSendAllSoundOff(Mixer *self, int midiChannelNumber)
{
    uint8_t bytes[3] = {0xB0 | midiChannelNumber, 0x78, 0x00};
    MixerChannel *channel = NAArrayGetValueAt(self->channels, midiChannelNumber);
    channel->source->send(channel->source, bytes, 3);
}

void MixerSendBank(Mixer *self, BankEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        uint8_t bytes[3];

        bytes[0] = 0xB0 | (0x0F & channel->midiNumber);
        bytes[1] = 0x00;
        bytes[2] = 0x7F & (event->bankNo >> 7);
        channel->source->send(channel->source, bytes, 3);

        bytes[1] = 0x20;
        bytes[2] = 0x7F & event->bankNo;
        channel->source->send(channel->source, bytes, 3);
    }
}

void MixerSendProgram(Mixer *self, ProgramEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        uint8_t bytes[3];

        bytes[0] = 0xC0 | (0x0F & channel->midiNumber);
        bytes[1] = event->programNo - 1;
        channel->source->send(channel->source, bytes, 2);
    }
}

void MixerSendVolume(Mixer *self, VolumeEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        channel->source->setVolume(channel->source, channel->midiNumber, event->value);
    }
}

void MixerSendPan(Mixer *self, PanEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        int value = Clip(event->value + 64, 0, 127);
        channel->source->setPan(channel->source, channel->midiNumber, value);
    }
}

void MixerSendChorus(Mixer *self, ChorusEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        channel->source->setChorusSend(channel->source, channel->midiNumber, event->value);
    }
}

void MixerSendReverb(Mixer *self, ReverbEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        channel->source->setReverbSend(channel->source, channel->midiNumber, event->value);
    }
}

void MixerSendExpression(Mixer *self, ExpressionEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        channel->source->setExpressionSend(channel->source, channel->midiNumber, event->value);
    }
}

void MixerSendPitch(Mixer *self, PitchEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        uint16_t pitchBend = event->value + 8192;
        uint8_t bytes[3] = {0xE0 | (0x7F & channel->midiNumber), 0x7F & pitchBend, 0x7F & (pitchBend >> 7)};
        channel->source->send(channel->source, bytes, 3);
    }
}

void MixerSendSustain(Mixer *self, SustainEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        uint8_t bytes[3];

        bytes[0] = 0xB0 | (0x0F & channel->midiNumber);
        bytes[1] = 0x40;
        bytes[2] = 0x7F & event->value;
        channel->source->send(channel->source, bytes, 3);
    }
}

void MixerSendDetune(Mixer *self, DetuneEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        uint8_t bytes[3];

        bytes[0] = 0xB0 | (0x0F & channel->midiNumber);

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

void MixerSendPitchSense(Mixer *self, PitchSenseEvent *event)
{
    MixerChannel *channel = NAArrayGetValueAt(self->channels, event->channel - 1);
    if (channel->active) {
        uint8_t bytes[3];

        bytes[0] = 0xB0 | (0x0F & channel->midiNumber);

        bytes[1] = 101;
        bytes[2] = 0;
        channel->source->send(channel->source, bytes, 3);

        bytes[1] = 100;
        channel->source->send(channel->source, bytes, 3);

        bytes[1] = 6;
        bytes[2] = event->value;
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

void MixerSetLevelEnable(Mixer *self, bool enable)
{
    self->levelEnable = enable;

    NAIterator *iterator = NAMapGetIterator(self->sourceMap);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        MidiSource *source = entry->value;
        source->setLevelEnable(source, enable);
    }
}

Level MixerGetLevel(Mixer *self)
{
    return self->level;
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
                source->setLevelEnable(source, self->levelEnable);
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

    double chorusSend[count];
    double reverbSend[count];

    for (int i = 0; i < count; ++i) {
        chorusSend[i] = 0.0;
        reverbSend[i] = 0.0;
    }

    NAIterator *iterator = NAArrayGetIterator(self->activeSources);
    while (iterator->hasNext(iterator)) {
        MidiSource *source = iterator->next(iterator);
        source->computeAudioSample(source, buffer, chorusSend, reverbSend, count);
    }

    for (int i = 0; i < count; ++i) {
        ChorusComputeSample(self->chorus, chorusSend[i], &buffer[i]);
        ReverbComputeSample(self->reverb, reverbSend[i], &buffer[i]);
    }

    if (self->levelEnable) {
        AudioSample valueLevel = {0, 0};

        for (int i = 0; i < count; i += UPDATE_THRESHOLD) {
            valueLevel.L = MAX(valueLevel.L, fabs(buffer[i].L));
            valueLevel.R = MAX(valueLevel.R, fabs(buffer[i].R));
        }

        self->level.L = Value2cB(valueLevel.L);
        self->level.R = Value2cB(valueLevel.R);
    }

    MixerProcessMessage(self);
}

static void _MixerNotifyChannelStatusChange(Mixer *self, Observer *observer, va_list argList)
{
    observer->callbacks->onChannelStatusChange(observer->receiver, va_arg(argList, MixerChannel *), va_arg(argList, MixerChannelStatusKind));
}

static void MixerNotifyChannelStatusChange(Mixer *self, MixerChannel *channel, MixerChannelStatusKind kind)
{
    NAArrayTraverseWithContext(self->observers, self, _MixerNotifyChannelStatusChange, channel, kind);
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
    case MidiSourceEventChangeGain:
    case MidiSourceEventChangeMasterVolume:
        break;
    case MidiSourceEventChangeVolume:
    case MidiSourceEventChangePan:
    case MidiSourceEventChangeChorusSend:
    case MidiSourceEventChangeReverbSend:
    case MidiSourceEventChangeExpressionSend:
    case MidiSourceEventChangePreset:
        {
            uint8_t midiChannelNumber = *((uint8_t *)arg1);
            MixerChannel *mixerChannel = NAArrayGetValueAt(self->channels, midiChannelNumber);

            switch (event) {
            case MidiSourceEventChangeVolume:
                MixerNotifyChannelStatusChange(self, mixerChannel, MixerChannelStatusKindVolume);
                break;
            case MidiSourceEventChangePan:
                MixerNotifyChannelStatusChange(self, mixerChannel, MixerChannelStatusKindPan);
                break;
            case MidiSourceEventChangeChorusSend:
                MixerNotifyChannelStatusChange(self, mixerChannel, MixerChannelStatusKindChorusSend);
                break;
            case MidiSourceEventChangeReverbSend:
                MixerNotifyChannelStatusChange(self, mixerChannel, MixerChannelStatusKindReverbSend);
                break;
            case MidiSourceEventChangeExpressionSend:
                MixerNotifyChannelStatusChange(self, mixerChannel, MixerChannelStatusKindExpressionSend);
                break;
            case MidiSourceEventChangePreset:
                MixerNotifyChannelStatusChange(self, mixerChannel, MixerChannelStatusKindPreset);
                break;
            default:
                break;
            }
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
        if (channel->active && !active) {
            MixerSendAllSoundOff(self, channel->midiNumber);
        }
        channel->active = active;
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
    MixerNotifyAvailableMidiSourceChange(receiver, descriptions);

    MidiSource *sourceToUnload = NAMapRemove(self->sourceMap, description);
    if (sourceToUnload) {
        MidiSourceDescription *defaultDescription = MidiSourceManagerGetDefaultDescription(manager);

        NAIterator *iterator = NAArrayGetIterator(self->channels);
        while (iterator->hasNext(iterator)) {
            MixerChannel *channel = iterator->next(iterator);
            if (channel->source == sourceToUnload) {
                MidiSource *source = NAMapGet(self->sourceMap, defaultDescription); 
                if (!source) {
                    source = MidiSourceManagerAllocMidiSource(manager, defaultDescription, self->sampleRate);
                    NAMapPut(self->sourceMap, defaultDescription, source);
                }

                channel->source = source;
                channel->description = defaultDescription;

                NAMessageQPost(self->msgQ, MixerMessageAttachSource, source);

                MixerNotifyChannelStatusChange(self, channel, MixerChannelStatusKindMidiSourceDescription);
            }
        }

        NAMessageQPost(self->msgQ, MixerMessageDetachSource, sourceToUnload);
    }
}

static void MixerMidiSourceManagerOnReorderMidiSourceDescriptions(void *receiver, NAArray *descriptions, NAArray *availableDescriptions)
{
    if (NAArrayIsEmpty(availableDescriptions)) {
        return;
    }

    Mixer *self = receiver;

    MixerNotifyAvailableMidiSourceChange(receiver, availableDescriptions);

    MidiSourceDescription *description = NAArrayGetValueAt(availableDescriptions, 0);
    NAIterator *iterator = NAArrayGetIterator(self->channels);
    while (iterator->hasNext(iterator)) {
        MixerChannel *channel = iterator->next(iterator);
        MixerChannelSetMidiSourceDescription(channel, description);
    }
}

static MidiSourceManagerObserverCallbacks MixerMidiSourceManagerObserverCallbacks = {
    MixerMidiSourceManagerOnLoadMidiSourceDescription,
    MixerMidiSourceManagerOnLoadAvailableMidiSourceDescription,
    MixerMidiSourceManagerOnUnloadMidiSourceDescription,
    MixerMidiSourceManagerOnUnloadAvailableMidiSourceDescription,
    MixerMidiSourceManagerOnReorderMidiSourceDescriptions
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
    return self->source->getPresetInfo(self->source, self->midiNumber);
}

Level MixerChannelGetLevel(MixerChannel *self)
{
    return self->source->getChannelLevel(self->source, self->midiNumber);
}

int MixerChannelGetVolume(MixerChannel *self)
{
    return self->source->getVolume(self->source, self->midiNumber);
}

int MixerChannelGetPan(MixerChannel *self)
{
    return self->source->getPan(self->source, self->midiNumber) - 64;
}

int MixerChannelGetChorusSend(MixerChannel *self)
{
    return self->source->getChorusSend(self->source, self->midiNumber);
}

int MixerChannelGetReverbSend(MixerChannel *self)
{
    return self->source->getReverbSend(self->source, self->midiNumber);
}

int MixerChannelGetExpressionSend(MixerChannel *self)
{
    return self->source->getExpressionSend(self->source, self->midiNumber);
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
    MidiSource *source = NAMapGet(self->mixer->sourceMap, description);

    if (!source) {
        MidiSourceManager *manager = MidiSourceManagerSharedInstance();
        source = MidiSourceManagerAllocMidiSource(manager, description, self->mixer->sampleRate);
        NAMapPut(self->mixer->sourceMap, description, source);
    }

    self->description = description;
    self->source = source;

    NAMessageQPost(self->mixer->msgQ, MixerMessageAttachSource, self->source);

    MixerNotifyChannelStatusChange(self->mixer, self, MixerChannelStatusKindMidiSourceDescription);
}

void MixerChannelSetPresetInfo(MixerChannel *self, PresetInfo *presetInfo)
{
    self->source->setPresetInfo(self->source, self->midiNumber, presetInfo);
}

void MixerChannelSetVolume(MixerChannel *self, int value)
{
    self->source->setVolume(self->source, self->midiNumber, value);
}

void MixerChannelSetPan(MixerChannel *self, int value)
{
    self->source->setPan(self->source, self->midiNumber, Clip(value + 64, 0, 127));
}

void MixerChannelSetChorusSend(MixerChannel *self, int value)
{
    self->source->setChorusSend(self->source, self->midiNumber, value);
}

void MixerChannelSetReverbSend(MixerChannel *self, int value)
{
    self->source->setReverbSend(self->source, self->midiNumber, value);
}

void MixerChannelSetExpressionSend(MixerChannel *self, int value)
{
    self->source->setExpressionSend(self->source, self->midiNumber, value);
}

void MixerChannelSetMute(MixerChannel *self, bool mute)
{
    self->mute = mute;
    MixerNotifyChannelStatusChange(self->mixer, self, MixerChannelStatusKindMute);
    MixerUpdateActiveChannles(self->mixer);
}

void MixerChannelSetSolo(MixerChannel *self, bool solo)
{
    self->solo = solo;
    MixerNotifyChannelStatusChange(self->mixer, self, MixerChannelStatusKindSolo);
    MixerUpdateActiveChannles(self->mixer);
}
