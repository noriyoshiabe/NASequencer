#include "NAMidi.h"

#include <pthread.h>
#include <NACFHelper.h>
#include "MessageQueue.h"
#include "Player.h"
#include "FSWatcher.h"
#include "ParseContext.h"

typedef enum _NAMidiMessageKind {
    NAMIDI_MSG_ADD_CONTEXT_VIEW,
    NAMIDI_MSG_SET_FILE,
    NAMIDI_MSG_START,
    NAMIDI_MSG_ON_FILE_CHANGED,
    NAMIDI_MSG_EXIT,
} NAMidiMessageKind;

struct _NAMidi {
    NAType _;
    pthread_t thread;
    Player *player;
    FSWatcher *watcher;
    CFMutableArrayRef contextViews;
    MessageQueue *msgQ;
    CFStringRef filepath;
    ParseContext *context;
};

static void __NAMidiParse(NAMidi *self)
{
    ParseContext *context = ParseContextParse(self->filepath);

    CFIndex count = CFArrayGetCount(self->contextViews);
    for (int i = 0; i < count; ++i) {
        void *view = (void *)CFArrayGetValueAtIndex(self->contextViews, i);
        ParseContextViewRender(view, context);
    }

    if (!context->error) {
        if (self->context) {
            NARelease(self->context);
        }
        self->context = context;
    }
    else {
        NARelease(context);
    }
}

static void __NAMidiPlaySequence(NAMidi *self)
{
    if (self->context) {
        PlayerSetSource(self->player, self->context->sequence);
        PlayerRewind(self->player);
        PlayerPlay(self->player);
    }
}

static void *__NAMidiRun(void *_self)
{
    NAMidi *self = _self;

    for (;;) {
        Message msg;
        MessageQueueWait(self->msgQ, &msg);

        switch (msg.kind) {
        case NAMIDI_MSG_ADD_CONTEXT_VIEW:
            CFArrayAppendValue(self->contextViews, msg.arg);
            NARelease(msg.arg);
            break;
        case NAMIDI_MSG_SET_FILE:
            // TODO switch file
            FSWatcherRegisterFilepath(self->watcher, msg.arg);
            self->filepath = msg.arg;
            break;
        case NAMIDI_MSG_START:
            __NAMidiParse(self);
            __NAMidiPlaySequence(self);
            break;
        case NAMIDI_MSG_ON_FILE_CHANGED:
            __NAMidiParse(self);
            __NAMidiPlaySequence(self);
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
    self->contextViews = CFArrayCreateMutable(NULL, 0, NACFArrayCallBacks);
    self->msgQ = MessageQueueCreate();

    pthread_create(&self->thread, NULL, __NAMidiRun, self);

    return self;
}

static void __NAMidiDestroy(void *_self)
{
    NAMidi *self = _self;

    Message msg = {NAMIDI_MSG_EXIT, NULL};
    MessageQueuePost(self->msgQ, &msg);

    pthread_join(self->thread, NULL);

    NARelease(self->player);
    NARelease(self->watcher);
    CFRelease(self->contextViews);

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

static void __NAMidiOnError(void *self, FSWatcher *fswatcher, int error, CFStringRef message)
{
    CFShow(message);
}

NADeclareVtbl(NAMidi, NAType, __NAMidiInit, __NAMidiDestroy, NULL, NULL, NULL, NULL, NULL);
NADeclareVtbl(NAMidi, FSWatcherListener, __NAMidiOnFileChanged, __NAMidiOnError);
NADeclareClass(NAMidi, NAType, FSWatcherListener);


void NAMidiAddContextView(NAMidi *self, void *contextView)
{
    Message msg = {NAMIDI_MSG_ADD_CONTEXT_VIEW, contextView};
    MessageQueuePost(self->msgQ, &msg);
}

void NAMidiSetFile(NAMidi *self, CFStringRef filepath)
{
    CFRetain(filepath);
    Message msg = {NAMIDI_MSG_SET_FILE, (void *)filepath};
    MessageQueuePost(self->msgQ, &msg);
}

void NAMidiStart(NAMidi *self)
{
    Message msg = {NAMIDI_MSG_START, NULL};
    MessageQueuePost(self->msgQ, &msg);
}
