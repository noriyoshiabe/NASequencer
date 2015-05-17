#pragma once

#include <stdint.h>

typedef struct _AudioSample {
    float L;
    float R;
} AudioSample;

typedef struct _AudioOut AudioOut;

typedef void (*AudioCallback)(void *receiver, uint32_t sampleRate, AudioSample *buffer, uint32_t length);

AudioOut *AudioOutSharedInstance();
void AudioOutRegisterCallback(AudioOut *self, AudioCallback function, void *receiver);
void AudioOutUnregisterCallback(AudioOut *self, AudioCallback function, void *receiver);
