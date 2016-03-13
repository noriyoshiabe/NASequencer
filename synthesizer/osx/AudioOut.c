#include "AudioOut.h"

#include <audiounit/AudioUnit.h>
#include <alloca.h>

typedef struct _AUAudioOut {
    AudioOut audioOut;
    AudioUnit defaultOutputUnit;
    void *receiver;
    AudioCallback callback;
    Float64 sampleRate;
} AUAudioOut;

static double AUAudioOutGetSampleRate(AudioOut *self);
static void AUAudioOutRegisterCallback(AudioOut *self, AudioCallback function, void *receiver);
static void AUAudioOutUnregisterCallback(AudioOut *self, AudioCallback function, void *receiver);

static OSStatus _RenderCallback(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp,
        UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
    AUAudioOut *self = inRefCon;

    AudioSample *buffer = ioData->mBuffers[0].mData;

    for (int i = 0; i < inNumberFrames; ++i) {
        buffer[i] = AudioSampleZero;
    }

    if (self->callback) {
        self->callback(self->receiver, buffer, inNumberFrames);
    }

    return noErr;
}

AudioOut *AudioOutCreate()
{
    AUAudioOut *self = calloc(1, sizeof(AUAudioOut));

    self->audioOut.getSampleRate = AUAudioOutGetSampleRate;
    self->audioOut.registerCallback = AUAudioOutRegisterCallback;
    self->audioOut.unregisterCallback = AUAudioOutUnregisterCallback;

    AudioComponentDescription cd;
	cd.componentType = kAudioUnitType_Output;
	cd.componentSubType = kAudioUnitSubType_HALOutput;
	cd.componentManufacturer = kAudioUnitManufacturer_Apple;
	cd.componentFlags = 0;
	cd.componentFlagsMask = 0;

	AudioComponent comp = AudioComponentFindNext(NULL, &cd);
	AudioComponentInstanceNew(comp, &self->defaultOutputUnit);

    AURenderCallbackStruct input;
	input.inputProc = _RenderCallback;
	input.inputProcRefCon = self;

	AudioUnitSetProperty(self->defaultOutputUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &input, sizeof(input));

    AudioStreamBasicDescription format;
    format.mSampleRate = 44100.0;
    format.mFormatID = kAudioFormatLinearPCM;
    format.mFormatFlags = kLinearPCMFormatFlagIsFloat;
    format.mBytesPerPacket = 2 * sizeof(float);
    format.mFramesPerPacket = 1;
    format.mBytesPerFrame = 2 * sizeof(float);
    format.mChannelsPerFrame = 2;
    format.mBitsPerChannel = 8 * sizeof(float);

    AudioUnitSetProperty(self->defaultOutputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &format, sizeof(AudioStreamBasicDescription));
    
    UInt32 bufferSize = 4096;
    AudioUnitSetProperty(self->defaultOutputUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Input, 0, &bufferSize, sizeof(UInt32));

	AudioUnitInitialize(self->defaultOutputUnit);
    AudioOutputUnitStart(self->defaultOutputUnit);

    return (AudioOut *)self;
}

void AudioOutDestroy(AudioOut *_self)
{
    AUAudioOut *self = (AUAudioOut *)_self;

    AudioOutputUnitStop(self->defaultOutputUnit);
    AudioUnitUninitialize(self->defaultOutputUnit);
    AudioComponentInstanceDispose(self->defaultOutputUnit);

    if (self->callback) {
        self->callback(self->receiver, NULL, 0);
    }

    free(self);
}

static double AUAudioOutGetSampleRate(AudioOut *_self)
{
    return 44100.0;
}

static void AUAudioOutRegisterCallback(AudioOut *_self, AudioCallback function, void *receiver)
{
    AUAudioOut *self = (AUAudioOut *)_self;

    self->callback = function; 
    self->receiver = receiver; 
}

static void AUAudioOutUnregisterCallback(AudioOut *_self, AudioCallback function, void *receiver)
{
    AUAudioOut *self = (AUAudioOut *)_self;

    self->callback = NULL;
    self->receiver = NULL;
}
