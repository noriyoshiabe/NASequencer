#pragma once

#include "MidiSource.h"
#include "NAArray.h"

typedef enum {
    MidiSourceDescriptionErrorNoError,
    MidiSourceDescriptionErrorFileNotFound,
    MidiSourceDescriptionErrorUnsupportedVersion,
    MidiSourceDescriptionErrorInvalidFileFormat,
    MidiSourceDescriptionErrorInvalidCRC,
} MidiSourceDescriptionError;

typedef struct _MidiSourceDescription {
    const char *name;
    const char *filepath;
    const bool available;
    const int gain;
    const int masterVolume;
    const MidiSourceDescriptionError error;
    const uint32_t crc32;
} MidiSourceDescription;

typedef struct _MidiSourceManagerObserverCallbacks {
    void (*onLoadMidiSourceDescription)(void *receiver, MidiSourceDescription *description);
    void (*onLoadAvailableMidiSourceDescription)(void *receiver, MidiSourceDescription *description);
    void (*onUnloadMidiSourceDescription)(void *receiver, MidiSourceDescription *description);
    void (*onUnloadAvailableMidiSourceDescription)(void *receiver, MidiSourceDescription *description);
    void (*onReorderMidiSourceDescriptions)(void *receiver, NAArray *descriptions, NAArray *availableDescriptions);
} MidiSourceManagerObserverCallbacks;

typedef struct _MidiSourceManager MidiSourceManager;

extern MidiSourceManager *MidiSourceManagerSharedInstance();
extern void MidiSourceManagerAddObserver(MidiSourceManager *self, void *receiver, MidiSourceManagerObserverCallbacks *callbacks);
extern void MidiSourceManagerRemoveObserver(MidiSourceManager *self, void *receiver);
extern bool MidiSourceManagerLoadMidiSourceDescriptionFromSoundFont(MidiSourceManager *self, const char *filepath);
extern void MidiSourceManagerUnloadMidiSourceDescription(MidiSourceManager *self, MidiSourceDescription *description);
extern bool MidiSourceManagerReloadMidiSourceDescription(MidiSourceManager *self, MidiSourceDescription *description);
extern void MidiSourceManagerDisableAvailableMidiSourceDescription(MidiSourceManager *self, MidiSourceDescription *description, MidiSourceDescriptionError error);
extern MidiSource *MidiSourceManagerAllocMidiSource(MidiSourceManager *self, MidiSourceDescription *description, double sampleRate);
extern void MidiSourceManagerDeallocMidiSource(MidiSourceManager *self, MidiSource *souce);
extern NAArray *MidiSourceManagerGetDescriptions(MidiSourceManager *self);
extern NAArray *MidiSourceManagerGetAvailableDescriptions(MidiSourceManager *self);
extern MidiSourceDescription *MidiSourceManagerGetDefaultDescription(MidiSourceManager *self);
extern void MidiSourceManagerSetGainForDescription(MidiSourceManager *self, MidiSourceDescription *description, int gain);
extern void MidiSourceManagerSetMasterVolumeForDescription(MidiSourceManager *self, MidiSourceDescription *description, int masterVolume);
extern void MidiSourceManagerSetReorderedDescriptions(MidiSourceManager *self, NAArray *descriptions);

static inline const char *MidiSourceDescriptionError2String(MidiSourceDescriptionError error)
{
#define CASE(error) case error: return &(#error[26])
    switch (error) {
    CASE(MidiSourceDescriptionErrorNoError);
    CASE(MidiSourceDescriptionErrorFileNotFound);
    CASE(MidiSourceDescriptionErrorUnsupportedVersion);
    CASE(MidiSourceDescriptionErrorInvalidFileFormat);
    CASE(MidiSourceDescriptionErrorInvalidCRC);
    default:
       break;
    }

    return "Unknown error";
#undef CASE
}
