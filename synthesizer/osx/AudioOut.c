#include "AudioOut.h"

#include <audiounit/AudioUnit.h>
#include <alloca.h>

typedef struct _Callback {
    void *receiver;
    AudioCallback function;
} Callback;

typedef struct _AUAudioOut {
    AudioOut audioOut;
    AudioUnit defaultOutputUnit;
    Callback *callbackList;
    int32_t callbackListLength;
    Float64 sampleRate;
} AUAudioOut;

static double AUAudioOutGetSampleRate(AudioOut *self);
static void AUAudioOutRegisterCallback(AudioOut *self, AudioCallback function, void *receiver);
static void AUAudioOutUnregisterCallback(AudioOut *self, AudioCallback function, void *receiver);

static AUAudioOut *_sharedInstance = NULL;

static OSStatus _RenderCallback(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp,
        UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
    AUAudioOut *self = inRefCon;

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

static AUAudioOut *AUAudioOutCreate()
{
    AUAudioOut *self = calloc(1, sizeof(AUAudioOut));

    self->audioOut.getSampleRate = AUAudioOutGetSampleRate;
    self->audioOut.registerCallback = AUAudioOutRegisterCallback;
    self->audioOut.unregisterCallback = AUAudioOutUnregisterCallback;

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
        _sharedInstance = AUAudioOutCreate();
    }
    return (AudioOut *)_sharedInstance;
}

static double AUAudioOutGetSampleRate(AudioOut *_self)
{
    AUAudioOut *self = (AUAudioOut *)_self;
    return (float)self->sampleRate;
}

static void AUAudioOutRegisterCallback(AudioOut *_self, AudioCallback function, void *receiver)
{
    AUAudioOut *self = (AUAudioOut *)_self;

    self->callbackList = realloc(self->callbackList, sizeof(Callback) * self->callbackListLength + 1);
    self->callbackList[self->callbackListLength].function = function; 
    self->callbackList[self->callbackListLength].receiver = receiver; 
    ++self->callbackListLength;
}

static void AUAudioOutUnregisterCallback(AudioOut *_self, AudioCallback function, void *receiver)
{
    AUAudioOut *self = (AUAudioOut *)_self;

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
