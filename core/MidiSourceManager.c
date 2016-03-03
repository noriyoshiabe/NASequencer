#include "MidiSourceManager.h"
#include "Synthesizer.h"
#include "NAMap.h"
#include "NASet.h"
#include "NATime.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct _MidiSourceManager {
    NAArray *observers;
    NAArray *descriptions;
    NAArray *availableDescriptions;
    NAMap *descriptionMap;
    NAMap *midiSourceMap;
};

typedef struct _MidiSourceDescriptionImpl {
    char *name;
    char *filepath;
    bool available;
    int gain;
    int masterVolume;
    MidiSourceDescriptionError error;
    SoundFont *sf;
    int sortIndex;
} MidiSourceDescriptionImpl;

typedef struct Observer {
    void *receiver;
    MidiSourceManagerObserverCallbacks *callbacks;
} Observer;

static MidiSourceDescriptionImpl *MidiSourceDescriptionImplCreate();
static void MidiSourceDescriptionImplDestroy(MidiSourceDescriptionImpl *self);

static MidiSource DefaultMidiSource;
static MidiSourceDescriptionImpl DefaultMidiSourceDescription;

static MidiSourceManager *_sharedInstance = NULL;

static MidiSourceManager *MidiSourceManagerCreate()
{
    MidiSourceManager *self = calloc(1, sizeof(MidiSourceManager));
    self->observers = NAArrayCreate(4, NULL);
    self->descriptions = NAArrayCreate(4, NULL);
    self->availableDescriptions = NAArrayCreate(4, NULL);
    self->descriptionMap = NAMapCreate(NAHashCString, NULL, NULL);
    self->midiSourceMap = NAMapCreate(NAHashAddress, NULL, NULL);
    return self;
}

static void MidiSourceManagerDestroy()
{
    MidiSourceManager *self = _sharedInstance;

    NATimeWaitUntil(self->observers, 10 * 1000, 50, (void *)NAArrayIsEmpty);

    NAArrayDestroy(self->observers);
    NAArrayDestroy(self->descriptions);
    NAArrayDestroy(self->availableDescriptions);
    NAMapDestroy(self->descriptionMap);
    NAMapDestroy(self->midiSourceMap);

    free(self);
}

MidiSourceManager *MidiSourceManagerSharedInstance()
{
    if (!_sharedInstance) {
        _sharedInstance = MidiSourceManagerCreate();
        atexit(MidiSourceManagerDestroy);
    }
    return _sharedInstance;
}

void MidiSourceManagerAddObserver(MidiSourceManager *self, void *receiver, MidiSourceManagerObserverCallbacks *callbacks)
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

void MidiSourceManagerRemoveObserver(MidiSourceManager *self, void *receiver)
{
    int index = NAArrayFindFirstIndex(self->observers, receiver, MixerObserverFindComparator);
    NAArrayApplyAt(self->observers, index, free);
    NAArrayRemoveAt(self->observers, index);
}

static void _MidiSourceManagerNotifyLoadMidiSourceDescription(MidiSourceManager *self, Observer *observer, va_list argList)
{
    observer->callbacks->onLoadMidiSourceDescription(observer->receiver, va_arg(argList, MidiSourceDescription *));
}

static void MidiSourceManagerNotifyLoadMidiSourceDescription(MidiSourceManager *self, MidiSourceDescriptionImpl *description)
{
    NAArrayTraverseWithContext(self->observers, self, _MidiSourceManagerNotifyLoadMidiSourceDescription, description);
}

static void _MidiSourceManagerNotifyLoadAvailableMidiSourceDescription(MidiSourceManager *self, Observer *observer, va_list argList)
{
    observer->callbacks->onLoadAvailableMidiSourceDescription(observer->receiver, va_arg(argList, MidiSourceDescription *));
}

static void MidiSourceManagerNotifyLoadAvailableMidiSourceDescription(MidiSourceManager *self, MidiSourceDescriptionImpl *description)
{
    NAArrayTraverseWithContext(self->observers, self, _MidiSourceManagerNotifyLoadAvailableMidiSourceDescription, description);
}

static void _MidiSourceManagerNotifyUnloadMidiSourceDescription(MidiSourceManager *self, Observer *observer, va_list argList)
{
    observer->callbacks->onUnloadMidiSourceDescription(observer->receiver, va_arg(argList, MidiSourceDescription *));
}

static void MidiSourceManagerNotifyUnloadMidiSourceDescription(MidiSourceManager *self, MidiSourceDescriptionImpl *description)
{
    NAArrayTraverseWithContext(self->observers, self, _MidiSourceManagerNotifyUnloadMidiSourceDescription, description);
}

static void _MidiSourceManagerNotifyUnloadAvailableMidiSourceDescription(MidiSourceManager *self, Observer *observer, va_list argList)
{
    observer->callbacks->onUnloadAvailableMidiSourceDescription(observer->receiver, va_arg(argList, MidiSourceDescription *));
}

static void MidiSourceManagerNotifyUnloadAvailableMidiSourceDescription(MidiSourceManager *self, MidiSourceDescriptionImpl *description)
{
    NAArrayTraverseWithContext(self->observers, self, _MidiSourceManagerNotifyUnloadAvailableMidiSourceDescription, description);
}

static void _MidiSourceManagerNotifyReorderMidiSourceDescriptions(MidiSourceManager *self, Observer *observer, va_list argList)
{
    observer->callbacks->onReorderMidiSourceDescriptions(observer->receiver, va_arg(argList, NAArray *), va_arg(argList, NAArray *));
}

static void MidiSourceManagerNotifyReorderMidiSourceDescriptions(MidiSourceManager *self, NAArray *descriptions, NAArray *availableDescriptions)
{
    NAArrayTraverseWithContext(self->observers, self, _MidiSourceManagerNotifyReorderMidiSourceDescriptions, descriptions, availableDescriptions);
}

bool MidiSourceManagerLoadMidiSourceDescriptionFromSoundFont(MidiSourceManager *self, const char *filepath)
{
    MidiSourceDescriptionImpl *description = NAMapGet(self->descriptionMap, (char *)filepath);

    bool created = false;

    if (!description) {
        description = MidiSourceDescriptionImplCreate();
        description->filepath = strdup(filepath);
        description->sortIndex = NAArrayCount(self->descriptions);
        created = true;
    }
        
    if (!description->sf) {
        SoundFontError error;
        description->sf = SoundFontRead(filepath, &error);

        if (!description->sf) {
            switch (error) {
            case SoundFontErrorFileNotFound:
                description->error = MidiSourceDescriptionErrorFileNotFound;
                break;
            case SoundFontErrorUnsupportedVersion:
                description->error = MidiSourceDescriptionErrorUnsupportedVersion;
                break;
            case SoundFontErrorInvalidFileFormat:
                description->error = MidiSourceDescriptionErrorInvalidFileFormat;
                break;
            }
        }

        if (description->sf) {
            free(description->name);
            description->name = strdup(description->sf->INAM);
            description->available = true;

            bool notifyUnloadDefault = 0 == NAArrayCount(self->availableDescriptions);
            NAArrayAppend(self->availableDescriptions, description);
            MidiSourceManagerNotifyLoadAvailableMidiSourceDescription(self, description);
            if (notifyUnloadDefault) {
                MidiSourceManagerNotifyUnloadAvailableMidiSourceDescription(self, &DefaultMidiSourceDescription);
            }
        }
    }

    if (created) {
        NAArrayAppend(self->descriptions, description);
        NAMapPut(self->descriptionMap, description->filepath, description);
        MidiSourceManagerNotifyLoadMidiSourceDescription(self, description);
    }

    return !!description->sf;
}

static int MidiSourceDescriptionFindComparator(const void *description1, const void *description2)
{
    return description1 - description2;
}

void MidiSourceManagerUnloadMidiSourceDescription(MidiSourceManager *self, MidiSourceDescription *description)
{
    int index;

    index = NAArrayFindFirstIndex(self->descriptions, description, MidiSourceDescriptionFindComparator);
    if (-1 != index) {
        NAArrayRemoveAt(self->descriptions, index);

        int count = NAArrayCount(self->descriptions);
        for (int i = index; i < count; ++i) {
            MidiSourceDescriptionImpl *_description = NAArrayGetValueAt(self->descriptions, i);
            _description->sortIndex = i;
        }

        MidiSourceManagerNotifyUnloadMidiSourceDescription(self, (MidiSourceDescriptionImpl *)description);
    }

    index = NAArrayFindFirstIndex(self->availableDescriptions, description, MidiSourceDescriptionFindComparator);
    if (-1 != index) {
        NAArrayRemoveAt(self->availableDescriptions, index);
        MidiSourceManagerNotifyUnloadAvailableMidiSourceDescription(self, (MidiSourceDescriptionImpl *)description);
    }

    if (!NAMapContainsKey(self->midiSourceMap, description)) {
        MidiSourceDescriptionImplDestroy((MidiSourceDescriptionImpl *)description);
    }
}

bool MidiSourceManagerReloadMidiSourceDescription(MidiSourceManager *self, MidiSourceDescription *_description)
{
    MidiSourceDescriptionImpl *description = (MidiSourceDescriptionImpl *)_description;

    SoundFontError error;
    description->sf = SoundFontRead(description->filepath, &error);

    if (!description->sf) {
        switch (error) {
        case SoundFontErrorFileNotFound:
            description->error = MidiSourceDescriptionErrorFileNotFound;
            break;
        case SoundFontErrorUnsupportedVersion:
            description->error = MidiSourceDescriptionErrorUnsupportedVersion;
            break;
        case SoundFontErrorInvalidFileFormat:
            description->error = MidiSourceDescriptionErrorInvalidFileFormat;
            break;
        }
    }

    if (description->sf) {
        free(description->name);
        description->name = strdup(description->sf->INAM);
        description->available = true;

        int i;
        int count = NAArrayCount(self->availableDescriptions);
        for (i = 0; i <= count; ++i) {
            if (count <= i) {
                NAArrayAppend(self->availableDescriptions, description);
                break;
            }

            MidiSourceDescriptionImpl *exsitance = NAArrayGetValueAt(self->availableDescriptions, i);
            if (description->sortIndex < exsitance->sortIndex) {
                NAArrayInsertAt(self->availableDescriptions, i, description);
                break;
            }
        }

        MidiSourceManagerNotifyLoadAvailableMidiSourceDescription(self, description);

        if (i < count) {
            MidiSourceManagerNotifyReorderMidiSourceDescriptions(self, self->descriptions, self->availableDescriptions);
        }
    }
    else {
        MidiSourceManagerNotifyLoadMidiSourceDescription(self, description);
    }

    return !!description->sf;
}

MidiSource *MidiSourceManagerAllocMidiSource(MidiSourceManager *self, MidiSourceDescription *_description, double sampleRate)
{
    MidiSourceDescriptionImpl *description = (MidiSourceDescriptionImpl *)_description;

    if (&DefaultMidiSourceDescription == description) {
        return &DefaultMidiSource;
    }

    if (description->sf) {
        NAArray *midiSources = NAMapGet(self->midiSourceMap, description);
        if (!midiSources) {
            midiSources = NAArrayCreate(4, NULL);
            NAMapPut(self->midiSourceMap, description, midiSources);
        }

        MidiSource *midiSource = (MidiSource *)SynthesizerCreate(description->sf, sampleRate);
        midiSource->setGain(midiSource, description->gain);
        midiSource->setMasterVolume(midiSource, description->masterVolume);
        NAArrayAppend(midiSources, midiSource);
        return midiSource;
    }

    return NULL;
}

void MidiSourceManagerDeallocMidiSource(MidiSourceManager *self, MidiSource *source)
{
    NAIterator *iterator = NAMapGetIterator(self->midiSourceMap);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        MidiSourceDescriptionImpl *description = entry->key;
        NAArray *sources = entry->value;

        int index = NAArrayFindFirstIndex(sources, source, NAArrayAddressComparator);
        if (-1 != index) {
            NAArrayRemoveAt(sources, index);
            source->destroy(source);

            if (NAArrayIsEmpty(sources)) {
                NAArrayDestroy(sources);
                iterator->remove(iterator);

                if (-1 == NAArrayFindFirstIndex(self->descriptions, description, NAArrayAddressComparator)) {
                    MidiSourceDescriptionImplDestroy(description);
                }
            }

            break;
        }
    }
}

NAArray *MidiSourceManagerGetDescriptions(MidiSourceManager *self)
{
    return self->descriptions;
}

NAArray *MidiSourceManagerGetAvailableDescriptions(MidiSourceManager *self)
{
    return self->availableDescriptions;
}

MidiSourceDescription *MidiSourceManagerGetDefaultDescription(MidiSourceManager *self)
{
    return 0 < NAArrayCount(self->availableDescriptions)
        ? NAArrayGetValueAt(self->availableDescriptions, 0)
        : (MidiSourceDescription *)&DefaultMidiSourceDescription;
}

void MidiSourceManagerSetGainForDescription(MidiSourceManager *self, MidiSourceDescription *_description, int gain)
{
    MidiSourceDescriptionImpl *description = (MidiSourceDescriptionImpl *)_description;
    description->gain = gain;

    NAArray *midiSources = NAMapGet(self->midiSourceMap, description);
    if (midiSources) {
        NAIterator *iterator = NAArrayGetIterator(midiSources);
        while (iterator->hasNext(iterator)) {
            MidiSource *midiSource = iterator->next(iterator);
            midiSource->setGain(midiSource, description->gain);
        }
    }
}

void MidiSourceManagerSetMasterVolumeForDescription(MidiSourceManager *self, MidiSourceDescription *_description, int masterVolume)
{
    MidiSourceDescriptionImpl *description = (MidiSourceDescriptionImpl *)_description;
    description->masterVolume = masterVolume;

    NAArray *midiSources = NAMapGet(self->midiSourceMap, description);
    if (midiSources) {
        NAIterator *iterator = NAArrayGetIterator(midiSources);
        while (iterator->hasNext(iterator)) {
            MidiSource *midiSource = iterator->next(iterator);
            midiSource->setMasterVolume(midiSource, description->masterVolume);
        }
    }
}

static int MidiSourceDescriptionSortComparator(const void *description1, const void *description2)
{
    MidiSourceDescriptionImpl **desc1 = (MidiSourceDescriptionImpl **)description1;
    MidiSourceDescriptionImpl **desc2 = (MidiSourceDescriptionImpl **)description2;
    return (*desc1)->sortIndex - (*desc2)->sortIndex;
}

void MidiSourceManagerSetReorderedDescriptions(MidiSourceManager *self, NAArray *descriptions)
{
    int count = NAArrayCount(descriptions);
    for (int i = 0; i < count; ++i) {
        MidiSourceDescriptionImpl *description = NAArrayGetValueAt(descriptions, i);
        description->sortIndex = i;
    }

    NAArraySort(self->descriptions, MidiSourceDescriptionSortComparator);
    NAArraySort(self->availableDescriptions, MidiSourceDescriptionSortComparator);

    MidiSourceManagerNotifyReorderMidiSourceDescriptions(self, self->descriptions, self->availableDescriptions);
}


static MidiSourceDescriptionImpl *MidiSourceDescriptionImplCreate()
{
    MidiSourceDescriptionImpl *self = calloc(1, sizeof(MidiSourceDescriptionImpl));
    self->name = strdup("N/A");
    self->gain = -100;
    return self;
}

static void MidiSourceDescriptionImplDestroy(MidiSourceDescriptionImpl *self)
{
    if (self->name) {
        free(self->name);
    }

    if (self->filepath) {
        free(self->filepath);
    }

    if (self->sf) {
        SoundFontDestroy(self->sf);
    }

    free(self);
}


static MidiSourceDescriptionImpl DefaultMidiSourceDescription = {
    "N/A", "N/A", true, 0, 0, MidiSourceDescriptionErrorNoError, NULL
};

static PresetInfo DefaultMidiSourcePresetInfo = {
    "N/A", 0, 0
};

static void DefaultMidiSourceSend(void *self, uint8_t *bytes, size_t length)
{
    printf("RX:");
    for (int i = 0; i < length; ++i) {
        printf(" %02X", bytes[i]);
    }
    printf("\n");
}

static bool DefaultMidiSourceIsAvailable(void *self) { return false; }
static void DefaultMidiSourceComputeAudioSample(void *self, AudioSample *buffer, uint32_t count) { }
static void DefaultMidiSourceRegisterCallback(void *self, MidiSourceCallback function, void *receiver) { }
static void DefaultMidiSourceUnregisterCallback(void *self, MidiSourceCallback function, void *receiver) { }
static void DefaultMidiSourceDestroy(void *self) { }
static const char *DefaultMidiSourceGetName(void *self) { return "N/A"; }
static int DefaultMidiSourceGetPresetCount(void *self) { return 1; }

static PresetInfo **DefaultMidiSourceGetPresetInfos(void *self)
{
    static PresetInfo *infos[] = {&DefaultMidiSourcePresetInfo};
    return infos;
}

static PresetInfo *DefaultMidiSourceGetPresetInfo(void *self, uint8_t channel)
{
    return &DefaultMidiSourcePresetInfo;
}

static void DefaultMidiSourceSetPresetInfo(void *self, uint8_t channel, PresetInfo *presetInfo) { }
static void DefaultMidiSourceSetLevelEnable(void *self, bool enable) { }
static Level DefaultMidiSourceGetMasterLevel(void *self) { return (Level){0, 0}; }
static Level DefaultMidiSourceGetChannelLevel(void *self, uint8_t channel) { return (Level){0, 0}; }
static void DefaultMidiSourceSetGain(void *self, int16_t cb) { }
static void DefaultMidiSourceSetMasterVolume(void *self, int16_t cb) { }
static void DefaultMidiSourceSetVolume(void *self, uint8_t channel, uint8_t value) { }
static void DefaultMidiSourceSetPan(void *self, uint8_t channel, uint8_t value) { }
static void DefaultMidiSourceSetChorusSend(void *self, uint8_t channel, uint8_t value) { }
static void DefaultMidiSourceSetReverbSend(void *self, uint8_t channel, uint8_t value) { }
static void DefaultMidiSourceSetExpressionSend(void *self, uint8_t channel, uint8_t value) { }
static int16_t DefaultMidiSourceGetGain(void *self) { return 0; }
static int16_t DefaultMidiSourceGetMasterVolume(void *self) { return 0; }
static uint8_t DefaultMidiSourceGetVolume(void *self, uint8_t channel) { return 0; }
static uint8_t DefaultMidiSourceGetPan(void *self, uint8_t channel) { return 0; }
static uint8_t DefaultMidiSourceGetChorusSend(void *self, uint8_t channel) { return 0; }
static uint8_t DefaultMidiSourceGetReverbSend(void *self, uint8_t channel) { return 0; }
static uint8_t DefaultMidiSourceGetExpressionSend(void *self, uint8_t channel) { return 0; }

static MidiSource DefaultMidiSource = {
    DefaultMidiSourceSend,
    DefaultMidiSourceIsAvailable,
    DefaultMidiSourceComputeAudioSample,
    DefaultMidiSourceRegisterCallback,
    DefaultMidiSourceUnregisterCallback,
    DefaultMidiSourceDestroy,
    DefaultMidiSourceGetName,
    DefaultMidiSourceGetPresetCount,
    DefaultMidiSourceGetPresetInfos,
    DefaultMidiSourceGetPresetInfo,
    DefaultMidiSourceSetPresetInfo,
    DefaultMidiSourceSetLevelEnable,
    DefaultMidiSourceGetMasterLevel,
    DefaultMidiSourceGetChannelLevel,
    DefaultMidiSourceSetGain,
    DefaultMidiSourceSetMasterVolume,
    DefaultMidiSourceSetVolume,
    DefaultMidiSourceSetPan,
    DefaultMidiSourceSetChorusSend,
    DefaultMidiSourceSetReverbSend,
    DefaultMidiSourceSetExpressionSend,
    DefaultMidiSourceGetGain,
    DefaultMidiSourceGetMasterVolume,
    DefaultMidiSourceGetVolume,
    DefaultMidiSourceGetPan,
    DefaultMidiSourceGetChorusSend,
    DefaultMidiSourceGetReverbSend,
    DefaultMidiSourceGetExpressionSend,
};
