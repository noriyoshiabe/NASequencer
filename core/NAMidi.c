#include "NAMidi.h"
#include "NAArray.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct Observer {
    void *receiver;
    NAMidiObserverCallbacks *callbacks;
} Observer;

struct _NAMidi {
    NAArray *observers;
};

NAMidi *NAMidiCreate()
{
    NAMidi *self = calloc(1, sizeof(NAMidi));
    self->observers = NAArrayCreate(1, sizeof(Observer));
    return self;
}

void NAMidiDestroy(NAMidi *self)
{
    NAArrayDestroy(self->observers);
    free(self);
}

void NAMidiAddObserver(NAMidi *self, void *receiver, NAMidiObserverCallbacks *callbacks)
{
    NAArrayAppend(self->observers, &(Observer){receiver, callbacks});
}

void NAMidiRemoveObserver(NAMidi *self, void *receiver)
{
    int count = NAArrayCount(self->observers);
    Observer *observers = NAArrayGetValues(self->observers);
    for (int i = 0; i < count; ++i) {
        if (observers[i].receiver == receiver) {
            NAArrayRemoveAtIndex(self->observers, i);
            break;
        }
    }
}

void NAMidiParse(NAMidi *self, const char *filepath)
{
    int count = NAArrayCount(self->observers);
    Observer *observers = NAArrayGetValues(self->observers);
    for (int i = 0; i < count; ++i) {
        observers[i].callbacks->onParseFinish(observers[i].receiver, NULL);
    }
}

Player *NAMidiGetPlayer(NAMidi *self)
{
    printf("called %s\n", __FUNCTION__);
    return NULL;
}

Mixer *NAMidiGetMixer(NAMidi *self)
{
    printf("called %s\n", __FUNCTION__);
    return NULL;
}
