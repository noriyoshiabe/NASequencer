#include "NAMidi.h"
#include "NAArray.h"
#include "NAMap.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct Observer {
    void *receiver;
    NAMidiObserverCallbacks *callbacks;
} Observer;

struct _NAMidi {
    NAMap *observers;
    NAArray *filepaths;
    Sequence *sequence;
};

NAMidi *NAMidiCreate()
{
    NAMidi *self = calloc(1, sizeof(NAMidi));
    self->observers = NAMapCreate(NULL, NULL, NULL);
    return self;
}

void NAMidiDestroy(NAMidi *self)
{
    NAMapDestroy(self->observers);
    free(self);
}

void NAMidiAddObserver(NAMidi *self, void *receiver, NAMidiObserverCallbacks *callbacks)
{
    NAMapPut(self->observers, receiver, callbacks);
}

void NAMidiRemoveObserver(NAMidi *self, void *receiver)
{
    NAMapRemove(self->observers, receiver);
}

static void NAMidiNotifyParseFinish(NAMidi *self, Sequence *sequence)
{
    uint8_t mapIteratorBuffer[NAMapIteratorSize];
    NAIterator *iterator = NAMapGetIterator(self->observers, mapIteratorBuffer);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        void *receiver = entry->key;
        NAMidiObserverCallbacks *callbacks = entry->value;
        callbacks->onParseFinish(receiver, self->sequence);
    }
}

static void NAMidiNotifyParseError(NAMidi *self, ParseError *error)
{
    uint8_t mapIteratorBuffer[NAMapIteratorSize];
    NAIterator *iterator = NAMapGetIterator(self->observers, mapIteratorBuffer);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        void *receiver = entry->key;
        NAMidiObserverCallbacks *callbacks = entry->value;
        callbacks->onParseError(receiver, error);
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
