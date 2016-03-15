#include "NAMidi.h"
#include "SequenceBuilderImpl.h"
#include "FSWatcher.h"
#include "AudioOut.h"
#include "NAArray.h"
#include "NAMap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Observer {
    void *receiver;
    NAMidiObserverCallbacks *callbacks;
} Observer;

struct _NAMidi {
    NAArray *observers;
    Sequence *sequence;
    ParseInfo *info;
    FSWatcher *watcher;
    bool watchEnable;
    bool changed;
    char *includePath;

    AudioOut *audioOut;
    Mixer *mixer;
    Player *player;
};

static FSWatcherCallbacks NAMidiFSWatcherCallbacks;

static void NAMidiStopFileWatch(NAMidi *self);

NAMidi *NAMidiCreate()
{
    NAMidi *self = calloc(1, sizeof(NAMidi));
    self->observers = NAArrayCreate(4, NULL);
    self->audioOut = AudioOutCreate();
    self->mixer = MixerCreate(self->audioOut);
    self->player = PlayerCreate(self->mixer);
    return self;
}

void NAMidiDestroy(NAMidi *self)
{
    PlayerDestroy(self->player);
    MixerDestroy(self->mixer);
    AudioOutDestroy(self->audioOut);

    NAMidiStopFileWatch(self);
    NAArrayTraverse(self->observers, free);
    NAArrayDestroy(self->observers);

    if (self->sequence) {
        SequenceRelease(self->sequence);
    }

    if (self->info) {
        ParseInfoRelease(self->info);
    }

    if (self->includePath) {
        free(self->includePath);
    }

    free(self);
}

void NAMidiAddObserver(NAMidi *self, void *receiver, NAMidiObserverCallbacks *callbacks)
{
    Observer *observer = malloc(sizeof(Observer));
    observer->receiver = receiver;
    observer->callbacks = callbacks;
    NAArrayAppend(self->observers, observer);
}

static int NAMidiObserverFindComparator(const void *receiver, const void *observer)
{
    return receiver - ((Observer *)observer)->receiver;
}

void NAMidiRemoveObserver(NAMidi *self, void *receiver)
{
    int index = NAArrayFindFirstIndex(self->observers, receiver, NAMidiObserverFindComparator);
    NAArrayApplyAt(self->observers, index, free);
    NAArrayRemoveAt(self->observers, index);
}

static void NAMidiNotifyBeforeParse(NAMidi *self, Observer *observer, va_list argList)
{
    observer->callbacks->onBeforeParse(observer->receiver, (bool)va_arg(argList, int));
}

static void NAMidiNotifyParseFinish(NAMidi *self, Observer *observer, va_list argList)
{
    observer->callbacks->onParseFinish(observer->receiver, va_arg(argList, Sequence *), va_arg(argList, ParseInfo *));
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
    NAMidiStopFileWatch(self);

    if (!self->info || NAArrayIsEmpty(self->info->filepaths)) {
        return;
    }

    self->watcher = FSWatcherCreate(&NAMidiFSWatcherCallbacks, self);

    NAIterator *iterator = NAArrayGetIterator(self->info->filepaths);
    while (iterator->hasNext(iterator)) {
        FSWatcherRegisterFilepath(self->watcher, iterator->next(iterator));
    }

    FSWatcherStart(self->watcher);
}

void NAMidiSetIncludePath(NAMidi *self, const char *includePath)
{
    if (self->includePath) {
        free(self->includePath);
    }
    self->includePath = strdup(includePath);
}

void NAMidiParse(NAMidi *self, const char *filepath)
{
    ParseInfo *info = NULL;

    NAArrayTraverseWithContext(self->observers, self, NAMidiNotifyBeforeParse, self->changed);

    SequenceBuilder *builder = SequenceBuilderCreate();
    Parser *parser = ParserCreate(builder, self->includePath);
    Sequence *sequence = ParserParseFile(parser, filepath, &info);
    ParserDestroy(parser);
    builder->destroy(builder);

    if (self->sequence) {
        SequenceRelease(self->sequence);
    }
    self->sequence = sequence;
    PlayerSetSequence(self->player, sequence);

    if (self->info) {
        ParseInfoRelease(self->info);
    }
    self->info = info;

    NAArrayTraverseWithContext(self->observers, self, NAMidiNotifyParseFinish, sequence, info);

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

Sequence *NAMidiGetSequence(NAMidi *self)
{
    return self->sequence;
}

ParseInfo *NAMidiGetParseInfo(NAMidi *self)
{
    return self->info;
}

static void NAMidiFSWatcherOnFileChanged(void *receiver, const char *changedFile)
{
    NAMidi *self = receiver;
    self->changed = true;
    NAMidiParse(self, NAArrayGetValueAt(self->info->filepaths, 0));
    self->changed = false;
}

static void NAMidiFSWatcherOnError(void *receiver, int error, const char *message)
{
    printf("%s: errno:%d message=%s\n", __FUNCTION__, error, message);
}

static FSWatcherCallbacks NAMidiFSWatcherCallbacks = {
    NAMidiFSWatcherOnFileChanged,
    NAMidiFSWatcherOnError,
};
