#include "Mixer.h"

#include <stdlib.h>

struct _Mixer {
};

Mixer *MixerCreate()
{
    return calloc(1, sizeof(Mixer));
}

void MixerDestroy(Mixer *self)
{
    free(self);
}
