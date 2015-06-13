#include "Synthesizer.h"
#include "MidiSource.h"
#include "SoundFont.h"
#include "Preset.h"

#include <stdlib.h>
#include <string.h>

typedef struct _Voice {
    int64_t startedAt;
    uint8_t noteNo;
} Voice;

struct _Synthesizer {
    MidiSource srcVtbl;
    char *filepath;
    SoundFont *sf;
    Preset **presets;
    int presetCount;
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

    ParsePresets(self->sf, &self->presets, &self->presetCount);

    return self;
}

void SynthesizerDestroy(Synthesizer *self)
{
    for (int i = 0; i < self->presetCount; ++i) {
        PresetDestroy(self->presets[i]);
    }

    if (self->presets) {
        free(self->presets);
    }

    SoundFontDestroy(self->sf);

    free(self->filepath);
    free(self);
}

void SynthesizerDump(Synthesizer *self)
{
    for (int i = 0; i < self->presetCount; ++i) {
        PresetDump(self->presets[i]);
    }
}
