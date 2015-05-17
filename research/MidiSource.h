#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct _MidiSource {
    void (*send)(void *self, uint8_t *bytes, size_t length);
    bool (*isAvailable)(void *self);
    bool (*hasProperty)(void *self, const char *name);
    void (*setProperty)(void *self, const char *name, const void *value);
    void (*getProperty)(void *self, const char *name, void *value);
} MidiSource;
