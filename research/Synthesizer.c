#include "Synthesizer.h"
#include "MidiSource.h"
#include "SoundFont.h"

#include <stdlib.h>
#include <string.h>

typedef struct _Preset Preset;
typedef struct _Instrument Instrument;
typedef struct _Generator Generator;
typedef struct _Sample Sample;
typedef struct _Voice Voice;

struct _Preset {
    const char *name;
    uint16_t midiPresetNumber;
    uint16_t bankNo;

    Generator generator;

    Instrument *instruments;
    int instrumentsCount;
};

struct _Instrument {
    const char *name;
    Generator generator;

    struct {
        Sample L;
        Sample R;
    } sample;

    Preset *preset;
};

struct _Generator {
    struct {
        uint8_t low;
        uint8_t high;
    } keyRange;

    struct {
        uint8_t low;
        uint8_t high;
    } velocityRange;
};

struct _Sample {
    const char *name;
    uint32_t start;
    uint32_t startLoop;
    uint32_t endLoop;
    uint32_t end;
    uint32_t sampleRate;
    uint8_t originalPitch;
    int8_t pitchCorrection;
};

struct _Voice {
    Instrument *instrument;
    int64_t startedAt;
    uint8_t noteNo;
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
