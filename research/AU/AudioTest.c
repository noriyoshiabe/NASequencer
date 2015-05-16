#include <audiounit/AudioUnit.h>

#include <stdlib.h>
#include <pthread.h>

typedef struct _AudioCallback {
    AudioUnit defaultOutputUnit;
} AudioCallback;


AudioCallback *AudioCallbackCreate(AURenderCallback inputProc, void *inputProcRefCon)
{
    AudioCallback *self = calloc(1, sizeof(AudioCallback));

    AudioComponentDescription cd;
	cd.componentType = kAudioUnitType_Output;
	cd.componentSubType = kAudioUnitSubType_DefaultOutput;
	cd.componentManufacturer = kAudioUnitManufacturer_Apple;
	cd.componentFlags = 0;
	cd.componentFlagsMask = 0;

	AudioComponent comp = AudioComponentFindNext(NULL, &cd);
	AudioComponentInstanceNew(comp, &self->defaultOutputUnit);

    AURenderCallbackStruct input;
	input.inputProc = inputProc;
	input.inputProcRefCon = inputProcRefCon;

	AudioUnitSetProperty(self->defaultOutputUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &input, sizeof(input));
	AudioUnitInitialize(self->defaultOutputUnit);

    return self;
}

bool AudioCallbackStart(AudioCallback *self)
{
    return noErr == AudioOutputUnitStart(self->defaultOutputUnit);
}


typedef struct _SineWave {
    float frequency;
    float phase;
} SineWave;

OSStatus RenderCallback(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp,
        UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
    SineWave *signWave = inRefCon;
    printf("---------- %.2f\n", signWave->frequency);
    float sinewaveFrequency = 440;
    float freq = signWave->frequency * 2 * M_PI / 44100;

    float *outL = ioData->mBuffers[0].mData;
    float *outR = ioData->mBuffers[1].mData;

    int i;
    for (i=0; i< inNumberFrames; i++){
        float wave = sin(signWave->phase) / 4;
        *outL++ = wave;
        *outR++ = wave;

        signWave->phase = signWave->phase + freq;
    }

    return noErr;
}



int main(int argc, char **argv)
{
    int base = 220;

    for (int i = 0; i < 40; ++i) {
        SineWave *sw = calloc(1, sizeof(SineWave));
        sw->frequency = base * (i + 1);
        AudioCallback *ac = AudioCallbackCreate(RenderCallback, sw);
        AudioCallbackStart(ac);
    }

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);

    return 0;
}
