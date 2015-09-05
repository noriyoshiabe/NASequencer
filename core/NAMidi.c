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
    NAArray *observers;
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
    self->observers = NAArrayCreate(4, NULL);
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
    NAArrayTraverse(self->observers, free);
    NAArrayDestroy(self->observers);
    free(self);
}

static int NAMidiObserverFindComparator(const void *receiver, const void *observer)
{
    return receiver - ((Observer *)observer)->receiver;
}

void NAMidiAddObserver(NAMidi *self, void *receiver, NAMidiObserverCallbacks *callbacks)
{
    Observer *observer = malloc(sizeof(Observer));
    observer->receiver = receiver;
    observer->callbacks = callbacks;
    NAArrayAppend(self->observers, observer);
}

void NAMidiRemoveObserver(NAMidi *self, void *receiver)
{
    int index = NAArrayFindFirstIndex(self->observers, receiver, NAMidiObserverFindComparator);
    NAArrayRemoveAtIndex(self->observers, index);
}

static void NAMidiNotifyParseFinish(NAMidi *self, Observer *observer, va_list argList)
{
    observer->callbacks->onParseFinish(observer->receiver, va_arg(argList, Sequence *));
}

static void NAMidiNotifyParseError(NAMidi *self, Observer *observer, va_list argList)
{
    observer->callbacks->onParseError(observer->receiver, va_arg(argList, ParseError *));
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
        NAArrayTraverseWithContext(self->observers, self, NAMidiNotifyParseError, &result.error);
    }
    else {
        if (self->sequence) {
            SequenceRelease(self->sequence);
        }

        self->sequence = result.sequence;

        NAArrayTraverseWithContext(self->observers, self, NAMidiNotifyParseFinish, self->sequence);
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
