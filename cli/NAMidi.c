#include "NAMidi.h"

#include <stdio.h>
#include <stdlib.h>

struct _NAMidi {
};

NAMidi *NAMidiCreate()
{
    NAMidi *self = calloc(1, sizeof(NAMidi));
    return self;
}

void NAMidiDestroy(NAMidi *self)
{
    free(self);
}

void NAMidiAddObserver(NAMidi *self, void *receiver, NAMidiObserverCallbacks *callbacks)
{
    printf("called %s\n", __FUNCTION__);
}

void NAMidiRemoveObserver(NAMidi *self, void *receiver)
{
    printf("called %s\n", __FUNCTION__);
}

void NAMidiParse(NAMidi *self, const char *filepath)
{
    printf("called %s\n", __FUNCTION__);
}
