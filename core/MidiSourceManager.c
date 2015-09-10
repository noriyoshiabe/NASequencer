#include "MidiSourceManager.h"
#include "Synthesizer.h"
#include "AudioOut.h"
#include "NAMap.h"
#include "NASet.h"

#include <stdlib.h>
#include <string.h>

struct _MidiSourceManager {
    NAArray *observers;
    NAArray *descriptions;
    NAArray *availableDescriptions;
    NAMap *descriptionMap;
    NAMap *sfSynthMap;
};

typedef struct _MidiSourceDescriptionImpl {
    char *name;
    char *filepath;
    bool available;
    MidiSourceDescriptionError error;
    SoundFont *sf;
} MidiSourceDescriptionImpl;

typedef struct Observer {
    void *receiver;
    MidiSourceManagerObserverCallbacks *callbacks;
} Observer;

static MidiSourceDescriptionImpl *MidiSourceDescriptionImplCreate();
static void MidiSourceDescriptionImplDestroy(MidiSourceDescriptionImpl *self);

static MidiSourceManager *_sharedInstance = NULL;

static MidiSourceManager *MidiSourceManagerCreate()
{
    MidiSourceManager *self = calloc(1, sizeof(MidiSourceManager));
    self->observers = NAArrayCreate(4, NULL);
    self->descriptions = NAArrayCreate(4, NULL);
    self->availableDescriptions = NAArrayCreate(4, NULL);
    self->descriptionMap = NAMapCreate(NAHashCString, NULL, NULL);
    self->sfSynthMap = NAMapCreate(NAHashAddress, NULL, NULL);

    // TODO default midi source ?
    return self;
}

MidiSourceManager *MidiSourceManagerSharedInstance()
{
    if (!_sharedInstance) {
        _sharedInstance = MidiSourceManagerCreate();
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
    NAArrayTraverseWithContext(self->observers, _MidiSourceManagerNotifyLoadMidiSourceDescription, self, description);
}

static void _MidiSourceManagerNotifyLoadAvailableMidiSourceDescription(MidiSourceManager *self, Observer *observer, va_list argList)
{
    observer->callbacks->onLoadAvailableMidiSourceDescription(observer->receiver, va_arg(argList, MidiSourceDescription *));
}

static void MidiSourceManagerNotifyLoadAvailableMidiSourceDescription(MidiSourceManager *self, MidiSourceDescriptionImpl *description)
{
    NAArrayTraverseWithContext(self->observers, _MidiSourceManagerNotifyLoadAvailableMidiSourceDescription, self, description);
}

void MidiSourceManagerLoadMidiSourceDescriptionFromSoundFont(MidiSourceManager *self, const char *filepath)
{
    MidiSourceDescriptionImpl *description = NAMapGet(self->descriptionMap, (char *)filepath);

    bool created = false;

    if (!description) {
        description = MidiSourceDescriptionImplCreate();
        description->filepath = strdup(filepath);
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

            NAMapPut(self->sfSynthMap, description->sf, NAArrayCreate(4, NULL));

            NAArrayAppend(self->availableDescriptions, description);
            MidiSourceManagerNotifyLoadAvailableMidiSourceDescription(self, description);
        }
    }

    if (created) {
        NAArrayAppend(self->descriptions, description);
        NAMapPut(self->descriptionMap, description->filepath, description);
        MidiSourceManagerNotifyLoadMidiSourceDescription(self, description);
    }
}

static void _MidiSourceManagerNotifyUnloadMidiSourceDescription(MidiSourceManager *self, Observer *observer, va_list argList)
{
    observer->callbacks->onUnloadMidiSourceDescription(observer->receiver, va_arg(argList, MidiSourceDescription *));
}

static void MidiSourceManagerNotifyUnloadMidiSourceDescription(MidiSourceManager *self, MidiSourceDescriptionImpl *description)
{
    NAArrayTraverseWithContext(self->observers, _MidiSourceManagerNotifyUnloadMidiSourceDescription, self, description);
}

static void _MidiSourceManagerNotifyUnloadAvailableMidiSourceDescription(MidiSourceManager *self, Observer *observer, va_list argList)
{
    observer->callbacks->onUnloadAvailableMidiSourceDescription(observer->receiver, va_arg(argList, MidiSourceDescription *));
}

static void MidiSourceManagerNotifyUnloadAvailableMidiSourceDescription(MidiSourceManager *self, MidiSourceDescriptionImpl *description)
{
    NAArrayTraverseWithContext(self->observers, _MidiSourceManagerNotifyUnloadAvailableMidiSourceDescription, self, description);
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
        NAArrayApplyAt(self->descriptions, index, MidiSourceDescriptionImplDestroy);
        NAArrayRemoveAt(self->descriptions, index);
        MidiSourceManagerNotifyUnloadMidiSourceDescription(self, (MidiSourceDescriptionImpl *)description);
    }

    index = NAArrayFindFirstIndex(self->availableDescriptions, description, MidiSourceDescriptionFindComparator);
    if (-1 != index) {
        NAArrayApplyAt(self->availableDescriptions, index, MidiSourceDescriptionImplDestroy);
        NAArrayRemoveAt(self->availableDescriptions, index);
        MidiSourceManagerNotifyUnloadAvailableMidiSourceDescription(self, (MidiSourceDescriptionImpl *)description);
    }
}

MidiSource *MidiSourceManagerAllocMidiSource(MidiSourceManager *self, MidiSourceDescription *_description)
{
    MidiSourceDescriptionImpl *description = (MidiSourceDescriptionImpl *)_description;
    if (description->sf) {
        NAArray *midiSources = NAMapGet(self->sfSynthMap, description->sf);
        MidiSource *ret = (MidiSource *)SynthesizerCreate(description->sf, AudioOutGetSampleRate(AudioOutSharedInstance()));
        NAArrayAppend(midiSources, ret);
        return ret;
    }

    return NULL;
}

static int MidiSourceFindComparator(const void *source1, const void *source2)
{
    return source1 - source2;
}

void MidiSourceManagerDeallocMidiSource(MidiSourceManager *self, MidiSource *souce)
{
    uint8_t iteratorBuffer[NAMapIteratorSize];
    NAIterator *iterator = NAMapGetIterator(self->sfSynthMap, iteratorBuffer);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        SoundFont *sf = entry->key;
        NAArray *sources = entry->value;

        int index = NAArrayFindFirstIndex(sources, sf, MidiSourceFindComparator);
        if (-1 != index) {
            MidiSource *source = NAArrayGetValueAt(sources, index);
            source->destroy(source);
            
            NAArrayRemoveAt(sources, index);
            if (NAArrayIsEmpty(sources)) {
                SoundFontDestroy(sf);
                NAArrayDestroy(sources);
                iterator->remove(iterator);
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

static MidiSourceDescriptionImpl *MidiSourceDescriptionImplCreate()
{
    MidiSourceDescriptionImpl *self = calloc(1, sizeof(MidiSourceDescriptionImpl));
    self->name = strdup("N/A");
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

    free(self);
}
