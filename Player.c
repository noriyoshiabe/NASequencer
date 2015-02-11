#include "Player.h"

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <NACFHelper.h>
#include "MidiClient.h"
#include "MessageQueue.h"

typedef enum _PlayerState {
    PLAYER_STATE_INVALID,
    PLAYER_STATE_STOP,
    PLAYER_STATE_PLAYING,
    PLAYER_STATE_EXIT,
} PlayerState;

typedef enum _PlayerMessageKind {
    PLAYER_MSG_SET_SOURCE,
    PLAYER_MSG_PLAY,
    PLAYER_MSG_STOP,
    PLAYER_MSG_EXIT,
} PlayerMessageKind;

struct _Player {
    NAType _;
    pthread_t thread;
    PlayerState state;
    TimeTable *timeTable;
    CFArrayRef events;
    CFMutableArrayRef playing;
    MessageQueue *msgQ;
    MidiClient *client;
    uint64_t start;
    uint64_t offset;
    uint64_t current;
    int index;
};

static void setSource(Player *self, void *source)
{
    if (self->timeTable) {
        NARelease(self->timeTable);
    }

    if (self->events) {
        CFRelease(self->events);
    }

    if (!source) {
        self->timeTable = NULL;
        self->events = NULL;
    }
    else {
        CFArrayRef events;
        TimeTable *timeTable;

        if (NATypeOf(source, Sequence)) {
            events = ((Sequence *)source)->events;
            timeTable = ((Sequence *)source)->timeTable;
        }
        else if (NATypeOf(source, Pattern)) {
            events = ((Pattern *)source)->events;
            timeTable = ((Pattern *)source)->timeTable;
        }
        else {
            NAPanic("Unexpected source type.");
        }

        self->events = CFRetain(events);
        self->timeTable = NARetain(timeTable);
    }
}

static void sendAllNoteOff(Player *self)
{
    uint8_t bytes[3] = {0, 0x7B, 0x00};
    for(int i = 0; i < 16; ++i) {
        bytes[0] = 0xB0 | (0x0F & i);
        MidiClientSend(self->client, bytes, sizeof(bytes));
    }
}

static uint64_t currentMicroSec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000 + tv.tv_usec;
}

static void changeState(Player *self, PlayerState next)
{
    switch (self->state) {
    case PLAYER_STATE_STOP:
        switch (next) {
        case PLAYER_STATE_PLAYING:
            self->start = currentMicroSec();
            self->offset = self->current;
            break;
        default:
            break;
        }
        break;
    case PLAYER_STATE_PLAYING:
        switch (next) {
        case PLAYER_STATE_STOP:
        case PLAYER_STATE_EXIT:
            sendAllNoteOff(self);
            CFArrayRemoveAllValues(self->playing);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    self->state = next;
}

static void sendNoteOff(Player *self, const NoteEvent *event)
{
}

static bool play(Player *self)
{
    uint64_t elapsed = currentMicroSec() - self->start;
    uint64_t prev = self->current;
    self->current = self->offset + elapsed;

    int32_t prevTick; // TODO
    int32_t currentTick; // TODO

    for (CFIndex i = CFArrayGetCount(self->playing); 0 < i; --i) {
        CFIndex idx = i - 1;
        const NoteEvent *event = CFArrayGetValueAtIndex(self->playing, idx);
        if (currentTick >= event->_.tick + event->gatetime) {
            sendNoteOff(self, event);
            CFArrayRemoveValueAtIndex(self->playing, idx);
        }
    }

    CFIndex eventsCount = CFArrayGetCount(self->events);
    for (; self->index < eventsCount; ++self->index) {
        const MidiEvent *event = CFArrayGetValueAtIndex(self->events, self->index);
        if (prevTick < event->tick && event->tick <= currentTick) {
            // TODO
        }
        else if (currentTick < event->tick) {
            break;
        }
    }

    CFIndex playingCount = CFArrayGetCount(self->playing);
    if (eventsCount <= self->index && 0 == playingCount) {
        changeState(self, PLAYER_STATE_STOP);
        return false;
    }
    else {
        return true;
    }
}

static void *__PlayerRun(void *_self)
{
    Player *self = _self;

    MidiClientOpen(self->client);
    changeState(self, PLAYER_STATE_STOP);

    for (;;) {
        Message msg;
        bool recv = false;
        
        if (PLAYER_STATE_STOP == self->state) {
            recv = MessageQueueWait(self->msgQ, &msg);
        }
        else {
            recv = MessageQueuePeek(self->msgQ, &msg);
        }

        if (recv) {
            switch (msg.kind) {
            case PLAYER_MSG_SET_SOURCE:
                setSource(self, msg.arg);
                NARelease(msg.arg);
                break;
            case PLAYER_MSG_PLAY:
                changeState(self, PLAYER_STATE_PLAYING);
                break;
            case PLAYER_MSG_STOP:
                changeState(self, PLAYER_STATE_STOP);
                break;
            case PLAYER_MSG_EXIT:
                changeState(self, PLAYER_STATE_EXIT);
                setSource(self, NULL);
                goto EXIT;
            }
        }

        if (PLAYER_STATE_PLAYING == self->state) {
            if (play(self)) {
                usleep(100);
            }
        }
    }

EXIT:
    MidiClientClose(self->client);
    return NULL;
}

static void *__PlayerInit(void *_self, ...)
{
    Player *self = _self;
    pthread_create(&self->thread, NULL, __PlayerRun, self);
    self->playing = CFArrayCreateMutable(NULL, 0, NACFArrayCallBacks);
    self->msgQ = MessageQueueCreate();
    self->client = NATypeNew(MidiClient);
    return self;
}

static void __PlayerDestroy(void *_self)
{
    Player *self = _self;

    Message msg = {PLAYER_MSG_EXIT, NULL};
    MessageQueuePost(self->msgQ, &msg);

    pthread_join(self->thread, NULL);

    CFRelease(self->playing);
    MessageQueueDestroy(self->msgQ);
    NARelease(self->client);
}

void PlayerSetSource(Player *self, void *source)
{
    NARetain(source);

    Message msg = {PLAYER_MSG_SET_SOURCE, source};
    MessageQueuePost(self->msgQ, &msg);
}

void PlayerPlay(Player *self)
{
    Message msg = {PLAYER_MSG_PLAY, NULL};
    MessageQueuePost(self->msgQ, &msg);
}

void PlayerStop(Player *self)
{
    Message msg = {PLAYER_MSG_STOP, NULL};
    MessageQueuePost(self->msgQ, &msg);
}
