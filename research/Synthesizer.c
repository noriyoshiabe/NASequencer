#include "Synthesizer.h"
#include "MidiSource.h"
#include "SoundFont.h"

#include <stdlib.h>
#include <string.h>

typedef struct _Preset Preset;
typedef struct _Instrument Instrument;
typedef struct _Voice Voice;

struct _Preset {
    const char *name;
    uint16_t midiPresetNumber;
    uint16_t bankNumber;

    uint16_t chorusEffectsSend;
    uint16_t reverbEffectsSend;

    int16_t pan;

    Instrument *instruments[128];
};

struct _Instrument {
    const char *name;

    uint16_t chorusEffectsSend;
    uint16_t reverbEffectsSend;

    int16_t pan;

    int8_t highestVelocity;
    int8_t lowestVelocity;
    int8_t fixedVelocity;

    int8_t fixedKeyNumber;
    
    struct {
        const char *name;

        uint32_t startLoop;
        uint32_t endLoop;
        uint32_t end;

        uint32_t sampleRate;
        uint8_t originalPitch;
        int8_t pitchCorrection;

        int16_t *L;
        int16_t *R;
        int8_t *L24;
        int8_t *R24;
    } sample;
};

struct _Voice {
    Preset *preset;
    int8_t keyNumber;
    float currentSamplePoint;
};


struct _Synthesizer {
    MidiSource srcVtbl;
    char *filepath;
    SoundFont *sf;
    Preset *presets;
    int presetsCount;
};

static void send(void *self, uint8_t *bytes, size_t length)
{
}

static bool isAvailable(void *self)
{
    return true;
}

static bool hasProperty(void *self, const char *name)
{
    return false;
}

static void setProperty(void *self, const char *name, const void *value)
{
}

static void getProperty(void *self, const char *name, void *value)
{
}

Synthesizer *SynthesizerCreate(const char *filepath)
{
    Synthesizer *self = calloc(1, sizeof(Synthesizer));

    self->srcVtbl.send = send;
    self->srcVtbl.isAvailable = isAvailable;
    self->srcVtbl.hasProperty = hasProperty;
    self->srcVtbl.setProperty = setProperty;
    self->srcVtbl.getProperty = getProperty;

    self->filepath = strdup(filepath);
    self->sf = SoundFontRead(filepath, NULL);

    return self;
}

void SynthesizerDestroy(Synthesizer *self)
{
    SoundFontDestroy(self->sf);
    free(self->filepath);
    free(self);
}
