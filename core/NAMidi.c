#include "NAMidi.h"

#include <pthread.h>
#include <NACFHelper.h>
#include "MessageQueue.h"
#include "FSWatcher.h"

typedef enum _NAMidiMessageKind {
    NAMIDI_MSG_ADD_OBSERVER,
    NAMIDI_MSG_SET_FILE,
    NAMIDI_MSG_PARSE,
    NAMIDI_MSG_PLAY,
    NAMIDI_MSG_STOP,
    NAMIDI_MSG_PLAY_PAUSE,
    NAMIDI_MSG_REWIND,
    NAMIDI_MSG_FORWARD,
    NAMIDI_MSG_BACKWARD,
    NAMIDI_MSG_ON_FILE_CHANGED,
    NAMIDI_MSG_EXIT,
} NAMidiMessageKind;

struct _NAMidi {
    NAType __;
    pthread_t thread;
    Player *player;
    FSWatcher *watcher;
    CFMutableArrayRef observers;
    MessageQueue *msgQ;
    CFStringRef filepath;
    ParseContext *context;
};

NADeclareAbstractClass(NAMidiObserver);

static void __NAMidiObserverOnParseFinished(NAMidi *self, ParseContext *context)
{
    CFIndex count = CFArrayGetCount(self->observers);
    for (int i = 0; i < count; ++i) {
        void *observer = (void *)CFArrayGetValueAtIndex(self->observers, i);
        void (*onParseFinished)(void *, NAMidi *, ParseContext *) = NAVtbl(observer, NAMidiObserver)->onParseFinished;
        if (onParseFinished) {
            onParseFinished(observer, self, context);
        }
    }
}

static void __NAMidiObserverOnPlayerContextChanged(NAMidi *self, PlayerContext *context)
{
    CFIndex count = CFArrayGetCount(self->observers);
    for (int i = 0; i < count; ++i) {
        void *observer = (void *)CFArrayGetValueAtIndex(self->observers, i);
        void (*onPlayerContextChanged)(void *, NAMidi *, PlayerContext *) = NAVtbl(observer, NAMidiObserver)->onPlayerContextChanged;
        if (onPlayerContextChanged) {
            onPlayerContextChanged(observer, self, context);
        }
    }
}

static void __NAMidiParse(NAMidi *self)
{
    ParseContext *context = ParseContextParse(self->filepath);
    __NAMidiObserverOnParseFinished(self, context);

    if (!context->error) {
        if (self->context) {
            NARelease(self->context);
        }
        self->context = context;

        PlayerSetSource(self->player, self->context->sequence);
    }
    else {
        NARelease(context);
    }
}

static void __NAMidiPlaySequence(NAMidi *self)
{
    if (self->context) {
        PlayerPlay(self->player);
    }
}

static void __NAMidiStop(NAMidi *self)
{
    if (self->context) {
        PlayerStop(self->player);
    }
}

static void __NAMidiPlayPauseSequence(NAMidi *self)
{
    if (self->context) {
        if (PlayerIsPlaying(self->player)) {
            PlayerStop(self->player);
        }
        else {
            PlayerPlay(self->player);
        }
    }
}

static void __NAMidiRewind(NAMidi *self)
{
    if (self->context) {
        PlayerRewind(self->player);
    }
}

static void __NAMidiForward(NAMidi *self)
{
    if (self->context) {
        PlayerForward(self->player);
    }
}

static void __NAMidiBackward(NAMidi *self)
{
    if (self->context) {
        PlayerBackward(self->player);
    }
}

static void *__NAMidiRun(void *_self)
{
    NAMidi *self = _self;

    for (;;) {
        Message msg;
        MessageQueueWait(self->msgQ, &msg);

        switch (msg.kind) {
        case NAMIDI_MSG_ADD_OBSERVER:
            CFArrayAppendValue(self->observers, msg.arg);
            NARelease(msg.arg);
            break;
        case NAMIDI_MSG_SET_FILE:
            FSWatcherRegisterFilepath(self->watcher, msg.arg);
            FSWatcherStart(self->watcher);
            self->filepath = msg.arg;
            break;
        case NAMIDI_MSG_PARSE:
            __NAMidiParse(self);
            break;
        case NAMIDI_MSG_PLAY:
            __NAMidiPlaySequence(self);
            break;
        case NAMIDI_MSG_STOP:
            __NAMidiStop(self);
            break;
        case NAMIDI_MSG_PLAY_PAUSE:
            __NAMidiPlayPauseSequence(self);
            break;
        case NAMIDI_MSG_REWIND:
            __NAMidiRewind(self);
            break;
        case NAMIDI_MSG_FORWARD:
            __NAMidiForward(self);
            break;
        case NAMIDI_MSG_BACKWARD:
            __NAMidiBackward(self);
            break;
        case NAMIDI_MSG_ON_FILE_CHANGED:
            __NAMidiParse(self);
            break;
        case NAMIDI_MSG_EXIT:
            goto EXIT;
        }
    }

EXIT:
    return NULL;
}

static void *__NAMidiInit(void *_self, ...)
{
    NAMidi *self = _self;

    self->player = NATypeNew(Player);
    self->watcher = NATypeNew(FSWatcher, self);
    self->observers = CFArrayCreateMutable(NULL, 0, NULL);
    self->msgQ = MessageQueueCreate();

    PlayerAddObserver(self->player, self);
    pthread_create(&self->thread, NULL, __NAMidiRun, self);

    return self;
}

static void __NAMidiDestroy(void *_self)
{
    NAMidi *self = _self;

    Message msg = {NAMIDI_MSG_EXIT, NULL};
    MessageQueuePost(self->msgQ, &msg);

    FSWatcherFinish(self->watcher);

    pthread_join(self->thread, NULL);

    NARelease(self->player);
    NARelease(self->watcher);
    CFRelease(self->observers);

    MessageQueueDestroy(self->msgQ);

    if (self->filepath) {
        CFRelease(self->filepath);
    }

    if (self->context) {
        NARelease(self->context);
    }
}

static void __NAMidiOnFileChanged(void *_self, FSWatcher *fswatcher, CFStringRef changedFile)
{
    NAMidi *self = _self;
    Message msg = {NAMIDI_MSG_ON_FILE_CHANGED, NULL};
    MessageQueuePost(self->msgQ, &msg);
}

static void __NAMidiOnPlayerContextChanged(void *self, Player *sender, PlayerContext *context)
{
    __NAMidiObserverOnPlayerContextChanged(self, context);
}

static void __NAMidiOnError(void *self, FSWatcher *fswatcher, int error, CFStringRef message)
{
    CFShow(message);
}

NADeclareVtbl(NAMidi, NAType, __NAMidiInit, __NAMidiDestroy, NULL, NULL, NULL, NULL, NULL);
NADeclareVtbl(NAMidi, PlayerObserver, __NAMidiOnPlayerContextChanged);
NADeclareVtbl(NAMidi, FSWatcherListener, __NAMidiOnFileChanged, __NAMidiOnError);
NADeclareClass(NAMidi, NAType, PlayerObserver, FSWatcherListener);

NAMidi *NAMidiCreate()
{
    return NATypeNew(NAMidi);
}

void NAMidiAddObserver(NAMidi *self, void *observer)
{
    NARetain(observer);
    Message msg = {NAMIDI_MSG_ADD_OBSERVER, observer};
    MessageQueuePost(self->msgQ, &msg);
}

void NAMidiSetFile(NAMidi *self, CFStringRef filepath)
{
    CFRetain(filepath);
    Message msg = {NAMIDI_MSG_SET_FILE, (void *)filepath};
    MessageQueuePost(self->msgQ, &msg);
}

void NAMidiParse(NAMidi *self)
{
    Message msg = {NAMIDI_MSG_PARSE, NULL};
    MessageQueuePost(self->msgQ, &msg);
}

void NAMidiPlay(NAMidi *self)
{
    Message msg = {NAMIDI_MSG_PLAY, NULL};
    MessageQueuePost(self->msgQ, &msg);
}

void NAMidiStop(NAMidi *self)
{
    Message msg = {NAMIDI_MSG_STOP, NULL};
    MessageQueuePost(self->msgQ, &msg);
}

void NAMidiPlayPause(NAMidi *self)
{
    Message msg = {NAMIDI_MSG_PLAY_PAUSE, NULL};
    MessageQueuePost(self->msgQ, &msg);
}

void NAMidiRewind(NAMidi *self)
{
    Message msg = {NAMIDI_MSG_REWIND, NULL};
    MessageQueuePost(self->msgQ, &msg);
}

void NAMidiForward(NAMidi *self)
{
    Message msg = {NAMIDI_MSG_FORWARD, NULL};
    MessageQueuePost(self->msgQ, &msg);
}

void NAMidiBackward(NAMidi *self)
{
    Message msg = {NAMIDI_MSG_BACKWARD, NULL};
    MessageQueuePost(self->msgQ, &msg);
}
