#include "Player.h"
#include "NAMessageQ.h"
#include "NAArray.h"

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/param.h>

typedef enum _PlayerMessage {
    PlayerMessageSetSequence,
    PlayerMessageStop,
    PlayerMessagePlay,
    PlayerMessageRewind,
    PlayerMessageForward,
    PlayerMessageBackward,
    PlayerMessageDestroy,
} PlayerMessage;

typedef struct Observer {
    void *receiver;
    PlayerObserverCallbacks *callbacks;
} Observer;

struct _Player {
    NAArray *observers;
    NAMessageQ *msgQ;
    pthread_t thread;
    bool exit;

    Mixer *mixer;
    Sequence *sequence;

    bool playing;
    Location location;
    int index;
    int tick;

    int64_t usec;
    int64_t start;
    int64_t offset;
};

static int64_t currentMicroSec();
static void *PlayerRun(void *self);
static void PlayerProcessMessage(Player *self, PlayerMessage message, void *data);
static void PlayerProcessEvent(Player *self, PlayerEvent event);
static void PlayerSupplyClock(Player *self);
static void PlayerScanEvents(Player *self, int32_t tick, int32_t prevTick);
static void PlayerSendNoteOn(Player *self, NoteEvent *event);
static void PlayerSendNoteOff(Player *self, NoteEvent *event);
static void PlayerScanNoteOff(Player *self, int tickFrom, int tickTo);
static void PlayerSendAllNoteOff();

Player *PlayerCreate(Mixer *mixer)
{
    Player *self = calloc(1, sizeof(Player));
    self->observers = NAArrayCreate(4, NULL);
    self->msgQ = NAMessageQCreate();
    self->mixer = mixer;
    self->location = LocationZero;
    pthread_create(&self->thread, NULL, PlayerRun, self);
    return self;
}

void PlayerDestroy(Player *self)
{
    self->playing = false;
    self->exit = true;
    NAMessageQPost(self->msgQ, PlayerMessageDestroy, NULL);
    pthread_join(self->thread, NULL);

    if (self->sequence) {
        SequenceRelease(self->sequence);
    }

    NAArrayTraverse(self->observers, free);
    NAArrayDestroy(self->observers);
    NAMessageQDestroy(self->msgQ);

    free(self);
}

void PlayerAddObserver(Player *self, void *receiver, PlayerObserverCallbacks *callbacks)
{
    Observer *observer = malloc(sizeof(Observer));
    observer->receiver = receiver;
    observer->callbacks = callbacks;
    NAArrayAppend(self->observers, observer);
}

static int PlayerObserverFindComparator(const void *receiver, const void *observer)
{
    return receiver - ((Observer *)observer)->receiver;
}

void PlayerRemoveObserver(Player *self, void *receiver)
{
    int index = NAArrayFindFirstIndex(self->observers, receiver, PlayerObserverFindComparator);
    NAArrayApplyAt(self->observers, index, free);
    NAArrayRemoveAt(self->observers, index);
}

void PlayerSetSequence(Player *self, Sequence *sequence)
{
    NAMessageQPost(self->msgQ, PlayerMessageSetSequence, SequenceRetain(sequence));
}

void PlayerPlay(Player *self)
{
    NAMessageQPost(self->msgQ, PlayerMessagePlay, NULL);
}

void PlayerStop(Player *self)
{
    NAMessageQPost(self->msgQ, PlayerMessageStop, NULL);
}

void PlayerPlayPause(Player *self)
{
    NAMessageQPost(self->msgQ, self->playing ? PlayerMessageStop : PlayerMessagePlay, NULL);
}

void PlayerRewind(Player *self)
{
    NAMessageQPost(self->msgQ, PlayerMessageRewind, NULL);
}

void PlayerForward(Player *self)
{
    NAMessageQPost(self->msgQ, PlayerMessageForward, NULL);
}

void PlayerBackWard(Player *self)
{
    NAMessageQPost(self->msgQ, PlayerMessageBackward, NULL);
}

static void *PlayerRun(void *_self)
{
    Player *self = _self;

    while (!self->exit) {
        bool (*recvMessage)(NAMessageQ *, NAMessage *) = self->playing ? NAMessageQPeek : NAMessageQWait;

        NAMessage msg;
        if (recvMessage(self->msgQ, &msg)) {
            PlayerProcessMessage(self, msg.kind, msg.data);
        }

        if (self->playing) {
            PlayerSupplyClock(self);
            usleep(100);
        }
    }

    return NULL;
}

static void PlayerNotifyEvent(Player *self, Observer *observer, va_list argList)
{
    observer->callbacks->onNotifyEvent(observer->receiver, *va_arg(argList, PlayerEvent *));
}

static void PlayerTriggerEvent(Player *self, PlayerEvent event)
{
    PlayerProcessEvent(self, event);
    NAArrayTraverseWithContext(self->observers, self, PlayerNotifyEvent, &event);
}

static void PlayerNotifyClock(Player *self, Observer *observer, va_list argList)
{
    observer->callbacks->onNotifyClock(observer->receiver,
            *va_arg(argList, int32_t *),
            *va_arg(argList, int32_t *),
            *va_arg(argList, int64_t *),
            *va_arg(argList, Location *));
}

static void PlayerUpdateClock(Player *self, int32_t tick, int32_t prevTick, int64_t usec, Location location)
{
    self->tick = tick;

    if (location.t != self->location.t
            || location.b != self->location.b
            || location.m != self->location.m) {
        self->location = location;
        NAArrayTraverseWithContext(self->observers, self, PlayerNotifyClock, &tick, &prevTick, &usec, &location);
    }
}

static void PlayerProcessMessage(Player *self, PlayerMessage message, void *data)
{
    switch (message) {
    case PlayerMessageSetSequence:
        if (self->sequence) {
            SequenceRelease(self->sequence);
        }
        self->sequence = data;
        break;
    case PlayerMessageStop:
        if (self->playing) {
            self->playing = false;
            PlayerTriggerEvent(self, PlayerEventStop);
        }
        break;
    case PlayerMessagePlay:
        if (!self->playing) {
            self->start = currentMicroSec();
            self->offset = self->usec;
            self->playing = true;
            PlayerTriggerEvent(self, PlayerEventPlay);
        }
        break;
    case PlayerMessageRewind:
        self->offset = 0;
        self->usec = 0;
        self->start = currentMicroSec();
        PlayerUpdateClock(self, 0, 0, 0, LocationZero);
        PlayerTriggerEvent(self, PlayerEventRewind);
        break;
    case PlayerMessageForward:
        {
            int32_t tick = TimeTableMicroSec2Tick(self->sequence->timeTable, self->usec);
            Location location = TimeTableTick2Location(self->sequence->timeTable, tick);
            location.m++;
            location.b = 1;
            location.t = 0;

            tick = TimeTableTickByMeasure(self->sequence->timeTable, location.m);
            tick = MIN(tick, TimeTableLength(self->sequence->timeTable));

            self->usec = TimeTableTick2MicroSec(self->sequence->timeTable, tick);
            self->offset = self->usec;
            self->start = currentMicroSec();

            PlayerUpdateClock(self, tick, tick, self->usec, location);
            PlayerTriggerEvent(self, PlayerEventForward);
        }
        break;
    case PlayerMessageBackward:
        {
            int32_t tick = TimeTableMicroSec2Tick(self->sequence->timeTable, self->usec);
            Location location = TimeTableTick2Location(self->sequence->timeTable, tick);
            location.m -= 1 == location.b ? 1 : 0;
            location.m = MAX(1, location.m);
            location.b = 1;
            location.t = 0;

            tick = TimeTableTickByMeasure(self->sequence->timeTable, location.m);

            self->usec = TimeTableTick2MicroSec(self->sequence->timeTable, tick);
            self->offset = self->usec;
            self->start = currentMicroSec();

            PlayerUpdateClock(self, tick, tick, self->usec, location);
            PlayerTriggerEvent(self, PlayerEventBackward);
        }
        break;
    case PlayerMessageDestroy:
        // NOP
        break;
    }
}

static void PlayerProcessEvent(Player *self, PlayerEvent event)
{
    switch (event) {
    case PlayerEventStop:
        PlayerSendAllNoteOff(self);
        break;
    case PlayerEventPlay:
        break;
    case PlayerEventRewind:
        PlayerSendAllNoteOff(self);
        self->index = 0;
        break;
    case PlayerEventForward:
        {
            PlayerSendAllNoteOff(self);

            int count = NAArrayCount(self->sequence->events);
            MidiEvent **events = NAArrayGetValues(self->sequence->events);
            for (; self->index < count; ++self->index) {
                if (self->tick <= events[self->index]->tick) {
                    break;
                }
            }
        }
        break;
    case PlayerEventBackward:
        {
            PlayerSendAllNoteOff(self);

            int count = NAArrayCount(self->sequence->events);
            MidiEvent **events = NAArrayGetValues(self->sequence->events);
            for (self->index = MAX(0, MIN(self->index - 1, count - 1)); 0 < self->index; --self->index) {
                if (self->tick > events[self->index]->tick) {
                    break;
                }
            }
        }
        break;
    case PlayerEventReachEnd:
        PlayerStop(self);
        PlayerRewind(self);
        break;
    }
}

static void PlayerSupplyClock(Player *self)
{
    int64_t elapsed = currentMicroSec() - self->start;
    int64_t prev = self->usec;
    self->usec = self->offset + elapsed;

    int32_t prevTick = TimeTableMicroSec2Tick(self->sequence->timeTable, prev);
    int32_t tick = TimeTableMicroSec2Tick(self->sequence->timeTable, self->usec);
    Location location = TimeTableTick2Location(self->sequence->timeTable, tick);

    PlayerScanEvents(self, tick, prevTick);
    PlayerUpdateClock(self, tick, prevTick, self->usec, location);

    if (TimeTableLength(self->sequence->timeTable) <= tick) {
        PlayerTriggerEvent(self, PlayerEventReachEnd);
    }
}

static void PlayerScanEvents(Player *self, int32_t tick, int32_t prevTick)
{
    // TODO
}

static void PlayerSendNoteOn(Player *self, NoteEvent *event)
{
    // TODO
}

static void PlayerSendNoteOff(Player *self, NoteEvent *event)
{
    // TODO
}

static void PlayerScanNoteOff(Player *self, int tickFrom, int tickTo)
{
    // TODO
}

static void PlayerSendAllNoteOff()
{
    // TODO
}

static int64_t currentMicroSec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000 + tv.tv_usec;
}
