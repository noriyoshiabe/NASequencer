#pragma once

#include <stdint.h>
#include "AudioSample.h"

typedef struct _AudioOut AudioOut;

typedef void (*AudioCallback)(void *receiver, AudioSample *buffer, uint32_t count);

AudioOut *AudioOutSharedInstance();
double AudioOutGetSampleRate(AudioOut *self);
void AudioOutRegisterCallback(AudioOut *self, AudioCallback function, void *receiver);
void AudioOutUnregisterCallback(AudioOut *self, AudioCallback function, void *receiver);
