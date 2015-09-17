#pragma once

#include <stdint.h>
#include "AudioSample.h"

typedef void (*AudioCallback)(void *receiver, AudioSample *buffer, uint32_t count);

typedef struct _AudioOut AudioOut;
struct _AudioOut {
    double (*getSampleRate)(AudioOut *self);
    void (*registerCallback)(AudioOut *self, AudioCallback function, void *receiver);
    void (*unregisterCallback)(AudioOut *self, AudioCallback function, void *receiver);
};

extern AudioOut *AudioOutSharedInstance();
