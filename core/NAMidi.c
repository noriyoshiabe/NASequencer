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
    NAArray *filepaths;
    Sequence *sequence;
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

static void NAMidiNotifyParseFinish(NAMidi *self, Sequence *sequence)
{
    int count = NAArrayCount(self->observers);
    Observer *observers = NAArrayGetValues(self->observers);
    for (int i = 0; i < count; ++i) {
        observers[i].callbacks->onParseFinish(observers[i].receiver, self->sequence);
    }
}

static void NAMidiNotifyParseError(NAMidi *self, ParseError *error)
{
    int count = NAArrayCount(self->observers);
    Observer *observers = NAArrayGetValues(self->observers);
    for (int i = 0; i < count; ++i) {
        observers[i].callbacks->onParseError(observers[i].receiver, error);
    }
}

void NAMidiParse(NAMidi *self, const char *filepath)
{
    ParseResult result;

    if (!ParserParseFile(filepath, &result)) {
        NAMidiNotifyParseError(self, &result.error);
    }
    else {
        if (self->sequence) {
            SequenceRelease(self->sequence);
        }

        if (self->filepaths) {
            NAArrayTraverse(self->filepaths, free);
            NAArrayDestroy(self->filepaths);
        }

        self->sequence = result.sequence;
        self->filepaths = result.filepaths;

        NAMidiNotifyParseFinish(self, self->sequence);
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
