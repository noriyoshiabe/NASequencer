#pragma once

#include <stdint.h>
#include "AudioSample.h"

typedef struct _AudioOut AudioOut;

typedef void (*AudioCallback)(void *receiver, AudioSample *buffer, uint32_t count);

extern AudioOut *AudioOutCreate();
extern void AudioOutDestroy(AudioOut *self);
extern double AudioOutGetSampleRate(AudioOut *self);
extern void AudioOutRegisterCallback(AudioOut *self, AudioCallback function, void *receiver);
extern void AudioOutUnregisterCallback(AudioOut *self, AudioCallback function, void *receiver);
