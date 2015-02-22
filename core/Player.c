#include "Player.h"

#include <pthread.h>
#include <sys/time.h>
#include <sys/param.h>
#include <unistd.h>
#include <NACFHelper.h>
#include "MidiClient.h"
#include "MessageQueue.h"

typedef enum _PlayerMessageKind {
    PLAYER_MSG_ADD_OBSERVER,
    PLAYER_MSG_SET_SOURCE,
    PLAYER_MSG_PLAY,
    PLAYER_MSG_STOP,
    PLAYER_MSG_REWIND,
    PLAYER_MSG_FORWARD,
    PLAYER_MSG_BACKWARD,
    PLAYER_MSG_EXIT,
} PlayerMessageKind;

struct _Player {
    NAType __;
    pthread_t thread;
    TimeTable *timeTable;
    CFArrayRef events;
    CFMutableArrayRef observers;
    MessageQueue *msgQ;
    MidiClient *client;
    int64_t start;
    int64_t offset;
    int index;
    PlayerContext context;
};

NADeclareAbstractClass(PlayerObserver);

static void __PlayerObserverOnPlayerContextChanged(Player *self)
{
    CFIndex count = CFArrayGetCount(self->observers);
    for (int i = 0; i < count; ++i) {
        void *observer = (void *)CFArrayGetValueAtIndex(self->observers, i);
        void (*onPlayerContextChanged)(void *, Player *, PlayerContext *) = NAVtbl(observer, PlayerObserver)->onPlayerContextChanged;
        if (onPlayerContextChanged) {
            onPlayerContextChanged(observer, self, &self->context);
        }
    }
}

void PlayerAddObserver(Player *self, void *observer)
{
    NARetain(observer);
    Message msg = {PLAYER_MSG_ADD_OBSERVER, observer};
    MessageQueuePost(self->msgQ, &msg);
}

static int64_t currentMicroSec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000 + tv.tv_usec;
}

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

        Location *location = &self->context.location;
        self->context.tick = TimeTableLocation2Tick(self->timeTable, location->m, location->b, location->t);
        self->offset = self->context.usec = TimeTableTick2MicroSec(self->timeTable, self->context.tick);
        self->start = currentMicroSec();
        TimeTableGetTempoByTick(self->timeTable, self->context.tick, &self->context.tempo);
        TimeTableGetTimeSignByTick(self->timeTable, self->context.tick, &self->context.numerator, &self->context.denominator);
        __PlayerObserverOnPlayerContextChanged(self);
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

static void __PlayerChangeState(Player *self, PlayerState next, bool silent)
{
    switch (self->context.state) {
    case PLAYER_STATE_STOP:
        switch (next) {
        case PLAYER_STATE_PLAYING:
            self->start = currentMicroSec();
            self->offset = self->context.usec;
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

    bool notify = self->context.state != next;
    self->context.state = next;
    if (!silent && notify) {
        __PlayerObserverOnPlayerContextChanged(self);
    }
}

static void __PlayerSendNoteOff(Player *self, const NoteEvent *event)
{
    uint8_t bytes[3] = {0x80 | (0x0F & (event->channel - 1)), event->noteNo, 0x00};
    MidiClientSend(self->client, bytes, sizeof(bytes));
}

static void __PlayerPlay(Player *self)
{
    int64_t elapsed = currentMicroSec() - self->start;
    int64_t prev = self->context.usec;
    self->context.usec = self->offset + elapsed;

    int32_t prevTick = TimeTableMicroSec2Tick(self->timeTable, prev);
    self->context.tick = TimeTableMicroSec2Tick(self->timeTable, self->context.usec);

    for (CFIndex i = CFArrayGetCount(self->context.playing); 0 < i; --i) {
        CFIndex idx = i - 1;
        const NoteEvent *event = CFArrayGetValueAtIndex(self->context.playing, idx);
        if (self->context.tick > event->__.tick + event->gatetime) {
            __PlayerSendNoteOff(self, event);
            CFArrayRemoveValueAtIndex(self->context.playing, idx);
        }
    }

    CFIndex eventsCount = CFArrayGetCount(self->events);
    for (; self->index < eventsCount; ++self->index) {
        MidiEvent *event = (MidiEvent *)CFArrayGetValueAtIndex(self->events, self->index);
        if (prevTick <= event->tick && event->tick < self->context.tick) {
            SequenceElementAccept(event, self);
        }
        else if (self->context.tick <= event->tick) {
            break;
        }
    }

    CFIndex playingCount = CFArrayGetCount(self->context.playing);
    if (eventsCount <= self->index && 0 == playingCount) {
        __PlayerChangeState(self, PLAYER_STATE_STOP, true);
        self->index = 0;
        self->context.usec = 0;
        self->context.tick = 0;
    }

    self->context.location = TimeTableTick2Location(self->timeTable, self->context.tick);
    TimeTableGetTempoByTick(self->timeTable, self->context.tick, &self->context.tempo);
    TimeTableGetTimeSignByTick(self->timeTable, self->context.tick, &self->context.numerator, &self->context.denominator);

    __PlayerObserverOnPlayerContextChanged(self);
}

static void __PlayerRewind(Player *self)
{
    __PlayerSendAllNoteOff(self);
    CFArrayRemoveAllValues(self->context.playing);

    self->index = 0;
    self->offset = 0;
    self->context.usec = 0;
    self->context.tick = 0;
    self->start = currentMicroSec();

    self->context.location = TimeTableTick2Location(self->timeTable, 0);
    TimeTableGetTempoByTick(self->timeTable, 0, &self->context.tempo);
    TimeTableGetTimeSignByTick(self->timeTable, 0, &self->context.numerator, &self->context.denominator);

    __PlayerObserverOnPlayerContextChanged(self);
}

static void __PlayerForward(Player *self)
{
    __PlayerSendAllNoteOff(self);
    CFArrayRemoveAllValues(self->context.playing);

    self->context.tick = TimeTableMicroSec2Tick(self->timeTable, self->context.usec);
    self->context.location = TimeTableTick2Location(self->timeTable, self->context.tick);
    Location *location = &self->context.location;
    location->m++;
    location->b = 1;
    location->t = 0;

    self->context.tick = TimeTableLocation2Tick(self->timeTable, location->m, location->b, location->t);
    self->context.usec = TimeTableTick2MicroSec(self->timeTable, self->context.tick);
    TimeTableGetTempoByTick(self->timeTable, self->context.tick, &self->context.tempo);
    TimeTableGetTimeSignByTick(self->timeTable, self->context.tick, &self->context.numerator, &self->context.denominator);

    CFIndex eventsCount = CFArrayGetCount(self->events);
    for (; self->index < eventsCount; ++self->index) {
        MidiEvent *event = (MidiEvent *)CFArrayGetValueAtIndex(self->events, self->index);
        if (self->context.tick <= event->tick) {
            break;
        }
    }

    self->offset = self->context.usec;
    self->start = currentMicroSec();

    __PlayerObserverOnPlayerContextChanged(self);
}

static void __PlayerBackward(Player *self)
{
    __PlayerSendAllNoteOff(self);
    CFArrayRemoveAllValues(self->context.playing);

    self->context.tick = TimeTableMicroSec2Tick(self->timeTable, self->context.usec);
    self->context.location = TimeTableTick2Location(self->timeTable, self->context.tick);
    Location *location = &self->context.location;

    if (1 == location->b) {
        location->m -= 1;
    }

    if (1 > location->m) {
        location->m = 1;
    }

    location->b = 1;
    location->t = 0;

    self->context.tick = TimeTableLocation2Tick(self->timeTable, location->m, location->b, location->t);
    self->context.usec = TimeTableTick2MicroSec(self->timeTable, self->context.tick);
    TimeTableGetTempoByTick(self->timeTable, self->context.tick, &self->context.tempo);
    TimeTableGetTimeSignByTick(self->timeTable, self->context.tick, &self->context.numerator, &self->context.denominator);

    for (self->index = MAX(0, self->index - 1); 0 < self->index; --self->index) {
        MidiEvent *event = (MidiEvent *)CFArrayGetValueAtIndex(self->events, self->index);
        if (self->context.tick >= event->tick) {
            break;
        }
    }
    
    self->offset = self->context.usec;
    self->start = currentMicroSec();

    __PlayerObserverOnPlayerContextChanged(self);
}

static void *PlayerRun(void *_self)
{
    Player *self = _self;

    MidiClientOpen(self->client);
    __PlayerChangeState(self, PLAYER_STATE_STOP, false);

    for (;;) {
        Message msg;
        bool recv = false;
        
        if (PLAYER_STATE_STOP == self->context.state) {
            recv = MessageQueueWait(self->msgQ, &msg);
        }
        else {
            recv = MessageQueuePeek(self->msgQ, &msg);
        }

        if (recv) {
            switch (msg.kind) {
            case PLAYER_MSG_ADD_OBSERVER:
                CFArrayAppendValue(self->observers, msg.arg);
                NARelease(msg.arg);
                break;
            case PLAYER_MSG_SET_SOURCE:
                __PlayerSetSource(self, msg.arg);
                NARelease(msg.arg);
                break;
            case PLAYER_MSG_PLAY:
                __PlayerChangeState(self, PLAYER_STATE_PLAYING, false);
                break;
            case PLAYER_MSG_STOP:
                __PlayerChangeState(self, PLAYER_STATE_STOP, false);
                break;
            case PLAYER_MSG_REWIND:
                __PlayerRewind(self);
                break;
            case PLAYER_MSG_FORWARD:
                __PlayerForward(self);
                break;
            case PLAYER_MSG_BACKWARD:
                __PlayerBackward(self);
                break;
            case PLAYER_MSG_EXIT:
                __PlayerChangeState(self, PLAYER_STATE_EXIT, false);
                __PlayerSetSource(self, NULL);
                goto EXIT;
            }
        }

        if (PLAYER_STATE_PLAYING == self->context.state) {
            __PlayerPlay(self);
            if (PLAYER_STATE_PLAYING == self->context.state) {
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

    self->context.playing = CFArrayCreateMutable(NULL, 0, NACFArrayCallBacks);
    self->observers = CFArrayCreateMutable(NULL, 0, NULL);
    self->msgQ = MessageQueueCreate();
    self->client = NATypeNew(MidiClient);

    Location *location = &self->context.location;
    location->b = 1;
    location->m = 1;
    location->t = 0;

    pthread_create(&self->thread, NULL, PlayerRun, self);

    return self;
}

static void __PlayerDestroy(void *_self)
{
    Player *self = _self;

    Message msg = {PLAYER_MSG_EXIT, NULL};
    MessageQueuePost(self->msgQ, &msg);

    pthread_join(self->thread, NULL);

    CFRelease(self->context.playing);
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

    CFArrayAppendValue(self->context.playing, elem);
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

void PlayerForward(Player *self)
{
    Message msg = {PLAYER_MSG_FORWARD, NULL};
    MessageQueuePost(self->msgQ, &msg);
}

void PlayerBackward(Player *self)
{
    Message msg = {PLAYER_MSG_BACKWARD, NULL};
    MessageQueuePost(self->msgQ, &msg);
}

bool PlayerIsPlaying(Player *self)
{
    return PLAYER_STATE_PLAYING == self->context.state;
}

const char *PlayerState2String(int state)
{
#define CASE(state) case state: return #state
    switch (state) {
    CASE(PLAYER_STATE_INVALID);
    CASE(PLAYER_STATE_STOP);
    CASE(PLAYER_STATE_PLAYING);
    CASE(PLAYER_STATE_EXIT);
    }
    return "Unknown token type";
#undef CASE
}

CFStringRef PlayerState2CFString(int state)
{
    return CFStringCreateWithCString(NULL, PlayerState2String(state), kCFStringEncodingUTF8);
}
