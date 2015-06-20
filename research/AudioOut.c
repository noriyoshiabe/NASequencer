#include "AudioOut.h"

#include <audiounit/AudioUnit.h>
#include <alloca.h>

typedef struct _Callback {
    void *receiver;
    AudioCallback function;
} Callback;

struct _AudioOut {
    AudioUnit defaultOutputUnit;
    Callback *callbackList;
    int32_t callbackListLength;
    Float64 sampleRate;
};

static AudioOut *_sharedInstance = NULL;

static OSStatus _RenderCallback(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp,
        UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
    AudioOut *self = inRefCon;

    float *outL = ioData->mBuffers[0].mData;
    float *outR = ioData->mBuffers[1].mData;

    for (int i = 0 ; i < inNumberFrames; ++i) {
        outL[i] = 0.0f;
        outR[i] = 0.0f;
    }

    size_t bufferSize = sizeof(AudioSample) * inNumberFrames;
    AudioSample *buffer = alloca(bufferSize);

    for (int i = 0; i < self->callbackListLength; ++i) {
        memset(buffer, 0, bufferSize);
        self->callbackList[i].function(self->callbackList[i].receiver, buffer, inNumberFrames);

        for (int j = 0; j < inNumberFrames; ++j) {
            outL[j] += buffer[j].L;
            outR[j] += buffer[j].R;
        }
    }

    return noErr;
}

static AudioOut *AudioOutCreate()
{
    AudioOut *self = calloc(1, sizeof(AudioOut));

    AudioComponentDescription cd;
	cd.componentType = kAudioUnitType_Output;
	cd.componentSubType = kAudioUnitSubType_DefaultOutput;
	cd.componentManufacturer = kAudioUnitManufacturer_Apple;
	cd.componentFlags = 0;
	cd.componentFlagsMask = 0;

	AudioComponent comp = AudioComponentFindNext(NULL, &cd);
	AudioComponentInstanceNew(comp, &self->defaultOutputUnit);

    UInt32 size = sizeof(Float64);
    AudioUnitGetProperty(self->defaultOutputUnit, kAudioUnitProperty_SampleRate, kAudioUnitScope_Output, 0, &self->sampleRate, &size);

    AURenderCallbackStruct input;
	input.inputProc = _RenderCallback;
	input.inputProcRefCon = self;

	AudioUnitSetProperty(self->defaultOutputUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &input, sizeof(input));
	AudioUnitInitialize(self->defaultOutputUnit);
    AudioOutputUnitStart(self->defaultOutputUnit);

    return self;
}

AudioOut *AudioOutSharedInstance()
{
    if (!_sharedInstance) {
        _sharedInstance = AudioOutCreate();
    }
    return _sharedInstance;
}

float AudioOutGetSampleRate(AudioOut *self)
{
    return (float)self->sampleRate;
}

void AudioOutRegisterCallback(AudioOut *self, AudioCallback function, void *receiver)
{
    self->callbackList = realloc(self->callbackList, sizeof(Callback) * self->callbackListLength + 1);
    self->callbackList[self->callbackListLength].function = function; 
    self->callbackList[self->callbackListLength].receiver = receiver; 
    ++self->callbackListLength;
}

void AudioOutUnregisterCallback(AudioOut *self, AudioCallback function, void *receiver)
{
    for (int i = 0; i < self->callbackListLength; ++i) {
        if (self->callbackList[i].function == function
                && self->callbackList[i].receiver == receiver) {

            size_t moveLength = self->callbackListLength - 1 - i;
            if (0 < moveLength) {
                memmove(&self->callbackList[i + 1], &self->callbackList[i], sizeof(Callback) * moveLength);
            }

            --self->callbackListLength;
            self->callbackList = realloc(self->callbackList, sizeof(Callback) * self->callbackListLength);
            break;
        }
    }
}
