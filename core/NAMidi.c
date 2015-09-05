#include "NAMidi.h"
#include "FSWatcher.h"
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
    FSWatcher *watcher;
    bool watchEnable;

    Mixer *mixer;
    Player *player;
};

static FSWatcherCallbacks NAMidiFSWatcherCallbacks;

static void NAMidiSetFilePaths(NAMidi *self, NAArray *filepaths);
static void NAMidiStopFileWatch(NAMidi *self);

NAMidi *NAMidiCreate()
{
    NAMidi *self = calloc(1, sizeof(NAMidi));
    self->observers = NAMapCreate(NULL, NULL, NULL);
    self->mixer = MixerCreate();
    self->player = PlayerCreate(self->mixer);
    return self;
}

void NAMidiDestroy(NAMidi *self)
{
    PlayerDestroy(self->player);
    MixerDestroy(self->mixer);

    NAMidiSetFilePaths(self, NULL);
    NAMidiStopFileWatch(self);
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

static void NAMidiSetFilePaths(NAMidi *self, NAArray *filepaths)
{
    if (self->filepaths) {
        NAArrayTraverse(self->filepaths, free);
        NAArrayDestroy(self->filepaths);
    }

    self->filepaths = filepaths;
}

static void NAMidiStopFileWatch(NAMidi *self)
{
    if (self->watcher) {
        FSWatcherDestroy(self->watcher);
        self->watcher = NULL;
    }
}

static void NAMidiStartFileWatch(NAMidi *self)
{
    if (!self->filepaths) {
        return;
    }

    if (self->watcher) {
        NAMidiStopFileWatch(self);
    }

    self->watcher = FSWatcherCreate(&NAMidiFSWatcherCallbacks, self);

    int count = NAArrayCount(self->filepaths);
    char **values = NAArrayGetValues(self->filepaths);
    for (int i = 0; i < count; ++i) {
        FSWatcherRegisterFilepath(self->watcher, values[i]);
    }

    FSWatcherStart(self->watcher);
}

void NAMidiParse(NAMidi *self, const char *filepath)
{
    ParseResult result = {};

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

    NAMidiSetFilePaths(self, result.filepaths);

    if (self->watchEnable) {
        NAMidiStartFileWatch(self);
    }
}

void NAMidiSetWatchEnable(NAMidi *self, bool watchEnable)
{
    if (!self->watchEnable && watchEnable) {
        NAMidiStartFileWatch(self);
    }
    else if (self->watchEnable && !watchEnable) {
        NAMidiStopFileWatch(self);
    }

    self->watchEnable = watchEnable;
}

Mixer *NAMidiGetMixer(NAMidi *self)
{
    return self->mixer;
}

Player *NAMidiGetPlayer(NAMidi *self)
{
    return self->player;
}

static void NAMidiFSWatcherOnFileChanged(void *receiver, const char *changedFile)
{
    NAMidi *self = receiver;
    char **filepaths = NAArrayGetValues(self->filepaths);
    NAMidiParse(self, filepaths[0]);
}

static void NAMidiFSWatcherOnError(void *receiver, int error, const char *message)
{
    printf("%s: message=%s\n", __FUNCTION__, message);
}

static FSWatcherCallbacks NAMidiFSWatcherCallbacks = {
    NAMidiFSWatcherOnFileChanged,
    NAMidiFSWatcherOnError,
};
