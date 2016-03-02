#pragma once

#include "AudioSample.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    MidiSourceEventChangeGain,
    MidiSourceEventChangeMasterVolume,
    MidiSourceEventChangeVolume,
    MidiSourceEventChangePan,
    MidiSourceEventChangeChorusSend,
    MidiSourceEventChangeReverbSend,
    MidiSourceEventChangeExpressionSend,
    MidiSourceEventChangePreset,
    MidiSourceEventChangeLevelMater,
} MidiSourceEvent;

typedef struct _PresetInfo {
    const char *name;
    uint16_t bankNo;
    uint8_t programNo;
} PresetInfo;

typedef struct _Level {
    int16_t L;
    int16_t R;
} Level;

typedef struct _MidiSource MidiSource;

typedef void (*MidiSourceCallback)(void *receiver, MidiSource *midiSrc, MidiSourceEvent event, void *arg1, void *arg2);

struct _MidiSource {
    void (*send)(void *self, uint8_t *bytes, size_t length);
    bool (*isAvailable)(void *self);

    void (*computeAudioSample)(void *self, AudioSample *buffer, uint32_t count);

    void (*registerCallback)(void *self, MidiSourceCallback function, void *receiver);
    void (*unregisterCallback)(void *self, MidiSourceCallback function, void *receiver);

    void (*destroy)(void *self);

    const char *(*getName)(void *self);

    int (*getPresetCount)(void *self);
    PresetInfo **(*getPresetInfos)(void *self);
    PresetInfo *(*getPresetInfo)(void *self, uint8_t channel);
    void (*setPresetInfo)(void *self, uint8_t channel, PresetInfo *presetInfo);

    void (*setLevelEnable)(void *self, bool enable);
    Level (*getMasterLevel)(void *self);
    Level (*getChannelLevel)(void *self, uint8_t channel);

    void (*setGain)(void *self, int16_t cb);
    void (*setMasterVolume)(void *self, int16_t cb);
    void (*setVolume)(void *self, uint8_t channel, uint8_t value);
    void (*setPan)(void *self, uint8_t channel, uint8_t value);
    void (*setChorusSend)(void *self, uint8_t channel, uint8_t value);
    void (*setReverbSend)(void *self, uint8_t channel, uint8_t value);
    void (*setExpressionSend)(void *self, uint8_t channel, uint8_t value);

    int16_t (*getGain)(void *self);
    int16_t (*getMasterVolume)(void *self);
    uint8_t (*getVolume)(void *self, uint8_t channel);
    uint8_t (*getPan)(void *self, uint8_t channel);
    uint8_t (*getChorusSend)(void *self, uint8_t channel);
    uint8_t (*getReverbSend)(void *self, uint8_t channel);
    uint8_t (*getExpressionSend)(void *self, uint8_t channel);
};

static inline const char *MidiSourceEvent2String(MidiSourceEvent event)
{
#define CASE(event) case event: return &(#event[15]);
    switch (event) {
    CASE(MidiSourceEventChangeGain);
    CASE(MidiSourceEventChangeMasterVolume);
    CASE(MidiSourceEventChangeVolume);
    CASE(MidiSourceEventChangePan);
    CASE(MidiSourceEventChangeChorusSend);
    CASE(MidiSourceEventChangeReverbSend);
    CASE(MidiSourceEventChangeExpressionSend);
    CASE(MidiSourceEventChangePreset);
    CASE(MidiSourceEventChangeLevelMater);
    }
#undef CASE
    return "MidiSourceEventUnknown";
}
