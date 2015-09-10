#include "MidiSourceManager.h"
#include "Synthesizer.h"
#include "NAMap.h"

#include <stdlib.h>
#include <string.h>

struct _MidiSourceManager {
    NAArray *observers;
    NAArray *descriptions;
    NAArray *availableDescriptions;
    NAMap *sfMap;
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
    self->sfMap = NAMapCreate(NAHashAddress, NULL, NULL);
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
    MidiSourceDescriptionImpl *description = MidiSourceDescriptionImplCreate();
    description->filepath = strdup(filepath);

    description->sf = NAMapGet(self->sfMap, description->filepath);
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
    }

    if (description->sf) {
        description->name = strdup(description->sf->INAM);
        description->available = true;

        NAArrayAppend(self->availableDescriptions, description);
        MidiSourceManagerNotifyLoadAvailableMidiSourceDescription(self, description);
    }
    
    NAArrayAppend(self->descriptions, description);
    MidiSourceManagerNotifyLoadMidiSourceDescription(self, description);
}

void MidiSourceManagerUnloadMidiSourceDescription(MidiSourceManager *self, MidiSourceDescription *description);
MidiSource *MidiSourceManagerAllocMidiSource(MidiSourceManager *self, MidiSourceDescription *description);
void MidiSourceManagerDeallocMidiSource(MidiSourceManager *self, MidiSource *souce);
NAArray *MidiSourceManagerGetDescriptions(MidiSourceManager *self);
NAArray *MidiSourceManagerGetAvailableDescriptions(MidiSourceManager *self);
