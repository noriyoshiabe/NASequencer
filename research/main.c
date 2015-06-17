#include "AudioOut.h"
#include "Synthesizer.h"
#include "MidiSource.h"
#include <unistd.h>
#include <math.h>

typedef struct _SineWave {
    float frequency;
    float phase;
} SineWave;

#if 0
static void _AudioCallback(void *receiver, uint32_t sampleRate, AudioSample *buffer, uint32_t length)
{
    SineWave *signWave = receiver;
    float freq = signWave->frequency * 2 * M_PI / sampleRate;

    for (int i = 0; i < length; ++i) {
        float wave = sin(signWave->phase) / 4;
        buffer[i].L = buffer[i].R = wave;
        signWave->phase += freq;
    }
}

#else

static void _AudioCallback(void *receiver, uint32_t sampleRate, AudioSample *buffer, uint32_t count)
{
    Synthesizer *synth = receiver;
    SynthesizerComputeAudioSample(synth, sampleRate, buffer, count);
}
#endif

#if 1
int main(int argc, char **argv)
{
    Synthesizer *synth = SynthesizerCreate(argv[1]);

    AudioOut *audioOut = AudioOutSharedInstance();
    AudioOutRegisterCallback(audioOut, _AudioCallback, synth);

    MidiSource *midiSrc = (MidiSource *)synth;
    midiSrc->send(midiSrc, NULL, 0);

    while (SynthesizerVoicingCount(synth)) {
        usleep(100);
    }

    AudioOutUnregisterCallback(audioOut, _AudioCallback, synth);
    SynthesizerDestroy(synth);
    return 0;
}

#else

int main(int argc, char **argv)
{
    AudioOut *audioOut = AudioOutSharedInstance();

    SineWave sw1 = {440, 0};
    AudioOutRegisterCallback(audioOut, _AudioCallback, &sw1);
    sleep(1);

    SineWave sw2 = {880, 0};
    AudioOutRegisterCallback(audioOut, _AudioCallback, &sw2);
    sleep(1);

    AudioOutUnregisterCallback(audioOut, _AudioCallback, &sw2);
    sleep(1);

    AudioOutUnregisterCallback(audioOut, _AudioCallback, &sw1);
    sleep(1);

    AudioOutRegisterCallback(audioOut, _AudioCallback, &sw1);
    sleep(1);

    return 0;
}
#endif
