#include "NAMidiSequenceBuilder.h"
#include "Sequence.h"

#include <stdlib.h>

typedef struct _NAMidiSequenceBuilder {
    SequenceBuilder interface;
    Sequence *sequence;
} NAMidiSequenceBuilder;

static void NAMidiSequenceBuilderDestroy(void *_self)
{
    NAMidiSequenceBuilder *self = _self;
    if (self->sequence) {
        SequenceRelease(self->sequence);
    }
    free(self);
}

static void *NAMidiSequenceBuilderBuild(void *_self)
{
    NAMidiSequenceBuilder *self = _self;
    Sequence *ret = self->sequence;
    self->sequence = NULL;
    return ret;
}

SequenceBuilder *NAMidiSequenceBuilderCreate()
{
    NAMidiSequenceBuilder *self = calloc(1, sizeof(NAMidiSequenceBuilder));
    self->interface.destroy = NAMidiSequenceBuilderDestroy;
    self->interface.build = NAMidiSequenceBuilderBuild;
    self->sequence = SequenceCreate();
    return (SequenceBuilder *)self;
}
