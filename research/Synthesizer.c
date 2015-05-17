#include "Synthesizer.h"
#include "MidiSource.h"
#include "SoundFont.h"

#include <stdlib.h>
#include <string.h>

struct _Synthesizer {
    MidiSource srcVtbl;
    char *filepath;
    SoundFont *sf;
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
