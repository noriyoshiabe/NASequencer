#include "AudioOut.h"
#include "Synthesizer.h"
#include "MidiSource.h"
#include "Channel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

struct termios orig_termios;

void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

void set_conio_terminal_mode()
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));

    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

int getch()
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}

static void _AudioCallback(void *receiver, AudioSample *buffer, uint32_t count)
{
    Synthesizer *synth = receiver;
    SynthesizerComputeAudioSample(synth, buffer, count);
}

int main(int argc, char **argv)
{
    AudioOut *audioOut = AudioOutSharedInstance();

    SoundFont *sf = SoundFontRead(argv[1], NULL);
    Synthesizer *synth = SynthesizerCreate(sf, AudioOutGetSampleRate(audioOut));

    AudioOutRegisterCallback(audioOut, _AudioCallback, synth);

    MidiSource *midiSrc = (MidiSource *)synth;

    int presetCount = midiSrc->getPresetCount(midiSrc);
    PresetList presetList[presetCount];
    midiSrc->getPresetList(midiSrc, presetList, presetCount);

    for (int i = 0; i < presetCount; ++i) {
        PresetList *preset = &presetList[i];
        printf("preset: name=%s bankNo=%d, programNo=%d\n", preset->name, preset->bankNo, preset->programNo);
    }

    set_conio_terminal_mode();

    int octave = 4;
    int presetIndex = 0;

    uint8_t bytes[3];

    bool noteon[128] = {0};
    bool sustain = false;

#if 0 // NRPN CC
    bytes[0] = 0xB0;
    bytes[1] = CC_NRPN_MSB; bytes[2] = 120; midiSrc->send(midiSrc, bytes, 3);
    bytes[1] = CC_NRPN_LSB; bytes[2] =  17; midiSrc->send(midiSrc, bytes, 3);
    bytes[1] = CC_DataEntry_MSB; bytes[2] = 0; midiSrc->send(midiSrc, bytes, 3);
#endif

#if 0 // RPN CC
    bytes[0] = 0xB0;
    bytes[1] = CC_RPN_MSB; bytes[2] = 0; midiSrc->send(midiSrc, bytes, 3);
    bytes[1] = CC_RPN_LSB; bytes[2] = 2; midiSrc->send(midiSrc, bytes, 3);
    bytes[1] = CC_DataEntry_MSB; bytes[2] = 64; midiSrc->send(midiSrc, bytes, 3);
#endif

#if 0 // Channel Pressure
    bytes[0] = 0xD0;
    bytes[1] = 64;
    midiSrc->send(midiSrc, bytes, 2);
#endif

    char c;
    while (3 != (c = getch())) {
        switch (c) {
        case 27:
            if (91 == getch()) {
                c = getch();
                switch (c) {
                case 'A': // up
                    {
                        ++presetIndex;
                        if (presetCount <= presetIndex) {
                            presetIndex = 0;
                        }

                        PresetList *preset = &presetList[presetIndex];

                        bytes[0] = 0xB0;
                        bytes[1] = 0x00;
                        bytes[2] = preset->bankNo >> 7 & 0x007F;
                        midiSrc->send(midiSrc, bytes, 3);

                        bytes[1] = 0x20;
                        bytes[2] = preset->bankNo & 0x007F;
                        midiSrc->send(midiSrc, bytes, 3);

                        bytes[0] = 0xC0;
                        bytes[1] = preset->programNo;
                        midiSrc->send(midiSrc, bytes, 2);

                        printf("preset change: index=%d name=%s bank:%d ptrogram:%d\r\n", presetIndex,
                                preset->name, preset->bankNo, preset->programNo);
                    }
                    break;
                case 'B': // down
                    {
                        --presetIndex;
                        if (0 > presetIndex) {
                            presetIndex = presetCount - 1;
                        }

                        PresetList *preset = &presetList[presetIndex];

                        bytes[0] = 0xB0;
                        bytes[1] = 0x00;
                        bytes[2] = preset->bankNo >> 7 & 0x007F;
                        midiSrc->send(midiSrc, bytes, 3);

                        bytes[1] = 0x20;
                        bytes[2] = preset->bankNo & 0x007F;
                        midiSrc->send(midiSrc, bytes, 3);

                        bytes[0] = 0xC0;
                        bytes[1] = preset->programNo;
                        midiSrc->send(midiSrc, bytes, 2);

                        printf("preset change: index=%d name=%s bank:%d ptrogram:%d\r\n", presetIndex,
                                preset->name, preset->bankNo, preset->programNo);
                    }
                    break;
                case 'C': // right
                    ++octave;
                    if (8 < octave) {
                        octave = 8;
                    }
                    else {
                        printf("octave=%d\r\n", octave);
                    }
                    break;
                case 'D': // left
                    --octave;
                    if (-2 > octave) {
                        octave = -2;
                    }
                    else {
                        printf("octave=%d\r\n", octave);
                    }
                    break;
                }
            }
            break;
        case 'S': // sustain
            sustain = !sustain;
            bytes[0] = 0xB0;
            bytes[1] = 64;
            bytes[2] = sustain ? 127 : 0;
            midiSrc->send(midiSrc, bytes, 3);
            printf("sustain=%s\r\n", sustain ? "on" : "off");
            break;
        case 'Q': // All sound Off
            bytes[0] = 0xB0;
            bytes[1] = 120;
            bytes[2] = 0;
            midiSrc->send(midiSrc, bytes, 3);
            printf("All sound off\r\n");
            break;
        case 'q': // All notes Off
            bytes[0] = 0xB0;
            bytes[1] = 123;
            bytes[2] = 0;
            midiSrc->send(midiSrc, bytes, 3);
            printf("All notes off\r\n");
            break;
        default:
            {
                int n = -99;

                switch (c) {
                case 'z': n = -3; break;
                case 's': n = -2; break;
                case 'x': n = -1; break;
                case 'c': n = 0; break;
                case 'f': n = 1; break;
                case 'v': n = 2; break;
                case 'g': n = 3; break;
                case 'b': n = 4; break;
                case 'n': n = 5; break;
                case 'j': n = 6; break;
                case 'm': n = 7; break;
                case 'k': n = 8; break;
                case ',': n = 9; break;
                case 'l': n = 10; break;
                case '.': n = 11; break;
                case '/': n = 12; break;
                }

                if (n != -99) {
                    int note = (octave + 2) * 12 + n;
                    if (0 < note && note < 128) {
                        if (noteon[note]) {
                            bytes[0] = 0x80;
                            bytes[1] = note;
                            bytes[2] = 0;
                            midiSrc->send(midiSrc, bytes, 3);

                            printf("note off: channel=0 noteNo=%d velocity=0\r\n", note);
                        }
                        else {
                            bytes[0] = 0x90;
                            bytes[1] = note;
                            bytes[2] = 127;
                            midiSrc->send(midiSrc, bytes, 3);

                            printf("note on: channel=0 noteNo=%d velocity=127\r\n", note);
                        }

                        noteon[note] = !noteon[note];
                    }
                }
            }
            break;
        }
    }

    getch(); /* consume the character */

    AudioOutUnregisterCallback(audioOut, _AudioCallback, synth);
    SynthesizerDestroy(synth);
    return 0;
}
