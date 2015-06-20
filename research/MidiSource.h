#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum {
    MidiSourcePropertyPresetMap,
    MidiSourcePropertyPresetCount,
} MidiSourceProperty;

typedef struct _MidiSource {
    void (*send)(void *self, uint8_t *bytes, size_t length);
    bool (*isAvailable)(void *self);
    bool (*hasProperty)(void *self, MidiSourceProperty property);
    void (*setProperty)(void *self, MidiSourceProperty property, const void *value);
    void (*getProperty)(void *self, MidiSourceProperty property, void *value);
} MidiSource;

typedef struct _PresetMap {
    char name[32];
    uint16_t bankNo;
    uint8_t programNo;
} PresetMap;
