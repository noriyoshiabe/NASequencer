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
    PLAYER_MSG_REWIND,
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

static void __PlayerSetSource(Player *self, void *source)
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
        SequenceElementAccept(source, self);
    }
}

static void __PlayerSendAllNoteOff(Player *self)
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

static void __PlayerChangeState(Player *self, PlayerState next)
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
            __PlayerSendAllNoteOff(self);
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

static void __PlayerSendNoteOff(Player *self, const NoteEvent *event)
{
    uint8_t bytes[3] = {0x80 | (0x0F & (event->channel - 1)), event->noteNo, 0x00};
    MidiClientSend(self->client, bytes, sizeof(bytes));
}

static bool __PlayerPlay(Player *self)
{
    uint64_t elapsed = currentMicroSec() - self->start;
    uint64_t prev = self->current;
    self->current = self->offset + elapsed;

    uint32_t prevTick = TimeTableMicroSec2Tick(self->timeTable, prev);
    uint32_t currentTick = TimeTableMicroSec2Tick(self->timeTable, self->current);

    for (CFIndex i = CFArrayGetCount(self->playing); 0 < i; --i) {
        CFIndex idx = i - 1;
        const NoteEvent *event = CFArrayGetValueAtIndex(self->playing, idx);
        if (currentTick > event->_.tick + event->gatetime) {
            __PlayerSendNoteOff(self, event);
            CFArrayRemoveValueAtIndex(self->playing, idx);
        }
    }

    CFIndex eventsCount = CFArrayGetCount(self->events);
    for (; self->index < eventsCount; ++self->index) {
        MidiEvent *event = (MidiEvent *)CFArrayGetValueAtIndex(self->events, self->index);
        if (prevTick <= event->tick && event->tick < currentTick) {
            SequenceElementAccept(event, self);
        }
        else if (currentTick <= event->tick) {
            break;
        }
    }

    CFIndex playingCount = CFArrayGetCount(self->playing);
    if (eventsCount <= self->index && 0 == playingCount) {
        __PlayerChangeState(self, PLAYER_STATE_STOP);
        return false;
    }
    else {
        return true;
    }
}

static void __PlayerRewind(Player *self)
{
    __PlayerSendAllNoteOff(self);
    CFArrayRemoveAllValues(self->playing);

    self->index = 0;
    self->offset = 0;
    self->current = 0;
    self->start = currentMicroSec();
}

static void *PlayerRun(void *_self)
{
    Player *self = _self;

    MidiClientOpen(self->client);
    __PlayerChangeState(self, PLAYER_STATE_STOP);

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
                __PlayerSetSource(self, msg.arg);
                NARelease(msg.arg);
                break;
            case PLAYER_MSG_PLAY:
                __PlayerChangeState(self, PLAYER_STATE_PLAYING);
                break;
            case PLAYER_MSG_STOP:
                __PlayerChangeState(self, PLAYER_STATE_STOP);
                break;
            case PLAYER_MSG_REWIND:
                __PlayerRewind(self);
                break;
            case PLAYER_MSG_EXIT:
                __PlayerChangeState(self, PLAYER_STATE_EXIT);
                __PlayerSetSource(self, NULL);
                goto EXIT;
            }
        }

        if (PLAYER_STATE_PLAYING == self->state) {
            if (__PlayerPlay(self)) {
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

    self->playing = CFArrayCreateMutable(NULL, 0, NACFArrayCallBacks);
    self->msgQ = MessageQueueCreate();
    self->client = NATypeNew(MidiClient);

    pthread_create(&self->thread, NULL, PlayerRun, self);

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

static void __PlayerVisitSequence(void *_self, Sequence *elem)
{
    Player *self = _self;
    self->events = CFRetain(elem->events);
    self->timeTable = NARetain(elem->timeTable);
}

static void __PlayerVisitPattern(void *_self, Pattern *elem)
{
    Player *self = _self;
    self->events = CFRetain(elem->events);
    self->timeTable = NARetain(elem->timeTable);
}

static void __PlayerVisitMarkerEvent(void *self, MarkerEvent *elem)
{
}

static void __PlayerVisitSoundSelectEvent(void *_self, SoundSelectEvent *elem)
{
    Player *self = _self;

    uint8_t bytes[3];
    bytes[0] = 0xB0 | (0x0F & (elem->channel - 1));
    bytes[1] = 0x00;
    bytes[2] = elem->msb;
    MidiClientSend(self->client, bytes, 3);

    bytes[1] = 0x20;
    bytes[2] = elem->lsb;
    MidiClientSend(self->client, bytes, 3);

    bytes[0] = 0xC0 | (0x0F & (elem->channel - 1));
    bytes[1] = elem->programNo;
    MidiClientSend(self->client, bytes, 2);
}

static void __PlayerVisitNoteEvent(void *_self, NoteEvent *elem)
{
    Player *self = _self;

    uint8_t bytes[3] = {0x90 | (0x0F & (elem->channel - 1)), elem->noteNo, elem->velocity};
    MidiClientSend(self->client, bytes, sizeof(bytes));

    CFArrayAppendValue(self->playing, elem);
}

NADeclareVtbl(Player, NAType,
        __PlayerInit,
        __PlayerDestroy,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        );

NADeclareVtbl(Player, SequenceVisitor,
        __PlayerVisitSequence,
        __PlayerVisitPattern,
        NULL,
        NULL,
        NULL,
        __PlayerVisitMarkerEvent,
        __PlayerVisitSoundSelectEvent,
        __PlayerVisitNoteEvent,
        );

NADeclareClass(Player, NAType, SequenceVisitor);


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

void PlayerRewind(Player *self)
{
    Message msg = {PLAYER_MSG_REWIND, NULL};
    MessageQueuePost(self->msgQ, &msg);
}

bool PlayerIsPlaying(Player *self)
{
    return PLAYER_STATE_PLAYING == self->state;
}
