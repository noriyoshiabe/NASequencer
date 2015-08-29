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
    self->observers = NAArrayCreate(1, sizeof(Observer), NULL);
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

static void NAMidiStopFileWatch(NAMidi *self)
{
    // TODO
    
    if (self->filepaths) {
        NAArrayTraverse(self->filepaths, free);
        NAArrayDestroy(self->filepaths);
        self->filepaths = NULL;
    }
}

static void NAMidiStartFileWatch(NAMidi *self, NAArray *filepaths)
{
    self->filepaths = filepaths;

    // TODO
}

void NAMidiParse(NAMidi *self, const char *filepath)
{
    ParseResult result;

    NAMidiStopFileWatch(self);

    if (!ParserParseFile(filepath, &result)) {
        NAMidiNotifyParseError(self, &result.error);
    }
    else {
        if (self->sequence) {
            SequenceRelease(self->sequence);
        }

        self->sequence = result.sequence;

        NAMidiNotifyParseFinish(self, self->sequence);
    }

    NAMidiStartFileWatch(self, result.filepaths);
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
