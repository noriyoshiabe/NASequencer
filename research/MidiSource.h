#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    MidiSourceEventChangeMasterVolume,
    MidiSourceEventChangeVolume,
    MidiSourceEventChangePan,
    MidiSourceEventChangeChorusSend,
    MidiSourceEventChangeReverbSend,
    MidiSourceEventChangeProgramNumber,
} MidiSourceEvent;

typedef struct _PresetList {
    char name[32];
    uint16_t bankNo;
    uint8_t programNo;
} PresetList;

typedef struct _MidiSource MidiSource;

typedef void (*MidiSourceCallback)(void *receiver, MidiSource *midiSrc, MidiSourceEvent event, void *arg);

struct _MidiSource {
    void (*send)(void *self, uint8_t *bytes, size_t length);
    bool (*isAvailable)(void *self);

    void (*registerCallback)(void *self, MidiSourceCallback callback, void *receiver);
    void (*ungisterCallback)(void *self, MidiSourceCallback callback, void *receiver);

    const char *(*getName)(void *self);

    int (*getPresetCount)(void *self);
    void (*getPresetList)(void *self, PresetList *presetList, size_t count);
    int (*getPresetIndex)(void *self, uint8_t channel);
    void (*setPresetIndex)(void *self, uint8_t channel, int index);

    int16_t (*getLastMasterLevel)(void *self);
    int16_t (*getLastLevel)(void *self, uint8_t channel);

    void (*setMasterVolume)(void *self, int16_t value);
    void (*setVolume)(void *self, uint8_t channel, uint8_t value);
    void (*setPan)(void *self, uint8_t channel, uint8_t value);
    void (*setChorusSend)(void *self, uint8_t channel, uint8_t value);
    void (*setReverbSend)(void *self, uint8_t channel, uint8_t value);

    int16_t (*getMasterVolume)(void *self);
    uint8_t (*getVolume)(void *self, uint8_t channel);
    uint8_t (*getPan)(void *self, uint8_t channel);
    uint8_t (*getChorusSend)(void *self, uint8_t channel);
    uint8_t (*getReverbSend)(void *self, uint8_t channel);
};
