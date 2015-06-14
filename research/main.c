#include "AudioOut.h"
#include "Synthesizer.h"
#include <unistd.h>
#include <math.h>

typedef struct _SineWave {
    float frequency;
    float phase;
} SineWave;

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

int main(int argc, char **argv)
{
    Synthesizer *synth = SynthesizerCreate(argv[1]);
    SynthesizerDestroy(synth);
    return 0;
}


#if 0
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
