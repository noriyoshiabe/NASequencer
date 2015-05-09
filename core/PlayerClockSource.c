#include "PlayerClockSource.h"
#include "MessageQueue.h"

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/param.h>

typedef enum _PlayerMessage {
    PlayerMessageSetTimeTable,
    PlayerMessageStop,
    PlayerMessagePlay,
    PlayerMessageRewind,
    PlayerMessageForward,
    PlayerMessageBackward,
    PlayerMessageDestroy,

    PlayerMessageSize,
} PlayerMessage;

struct _PlayerClockSource {
    PlayerClockSourceCallbacks *callbacks;
    void *receiver;

    MessageQueue *msgQ;
    pthread_t thread;
    bool exit;

    TimeTable *timeTable;

    bool playing;

    int64_t usec;
    int64_t start;
    int64_t offset;
};

static void *_PlayerClockSourceRun(void *_self);

PlayerClockSource *PlayerClockSourceCreate(PlayerClockSourceCallbacks *callbacks, void *receiver)
{
    PlayerClockSource *self = calloc(1, sizeof(PlayerClockSource));
    self->callbacks = callbacks;
    self->receiver = receiver;
    self->msgQ = MessageQueueCreate();
    pthread_create(&self->thread, NULL, _PlayerClockSourceRun, self);
    return self;
}

void PlayerClockSourceDestroy(PlayerClockSource *self)
{
    self->receiver = NULL;
    MessageQueuePost(self->msgQ, PlayerMessageDestroy, NULL);
}

void PlayerClockSourceSetTimeTable(PlayerClockSource *self, TimeTable *timeTable)
{
    MessageQueuePost(self->msgQ, PlayerMessageSetTimeTable, TimeTableRetain(timeTable));
}

void PlayerClockSourceStop(PlayerClockSource *self)
{
    MessageQueuePost(self->msgQ, PlayerMessageStop, NULL);
}

void PlayerClockSourcePlay(PlayerClockSource *self)
{
    MessageQueuePost(self->msgQ, PlayerMessagePlay, NULL);
}

void PlayerClockSourceRewind(PlayerClockSource *self)
{
    MessageQueuePost(self->msgQ, PlayerMessageRewind, NULL);
}

void PlayerClockSourceFoward(PlayerClockSource *self)
{
    MessageQueuePost(self->msgQ, PlayerMessageForward, NULL);
}

void PlayerClockSourceBackword(PlayerClockSource *self)
{
    MessageQueuePost(self->msgQ, PlayerMessageBackward, NULL);
}

static void (*functionTable[PlayerMessageSize])(PlayerClockSource *self, void *data) = {NULL};
static void _PlayerClockSourceSupplyClock(PlayerClockSource *self);

static void *_PlayerClockSourceRun(void *_self)
{
    PlayerClockSource *self = _self;

    while (!self->exit) {
        bool (*recvMessage)(MessageQueue *, Message *) = self->playing ? MessageQueuePeek : MessageQueueWait;

        Message msg;
        if (recvMessage(self->msgQ, &msg)) {
            functionTable[msg.kind](self, msg.data);
        }

        if (self->playing) {
            _PlayerClockSourceSupplyClock(self);
            usleep(100);
        }
    }

    MessageQueueDestroy(self->msgQ);
    free(self);

    return NULL;
}

static int64_t currentMicroSec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000 + tv.tv_usec;
}

static void _PlayerClockSourceNotifyEvent(PlayerClockSource *self, PlayerClockSourceEvent event)
{
    if (self->receiver) {
        self->callbacks->onNotifyEvent(self->receiver, event);
    }
}

static void _PlayerClockSourceNotifyClock(PlayerClockSource *self, int32_t tick, int32_t prevTick, int64_t usec, Location location)
{
    if (self->receiver) {
        self->callbacks->onNotifyClock(self->receiver, tick, prevTick, usec, location);
    }
}

static void _PlayerClockSourceSupplyClock(PlayerClockSource *self)
{
    int64_t elapsed = currentMicroSec() - self->start;
    int64_t prev = self->usec;
    self->usec = self->offset + elapsed;

    int32_t prevTick = TimeTableMicroSec2Tick(self->timeTable, prev);
    int32_t tick = TimeTableMicroSec2Tick(self->timeTable, self->usec);
    Location location = TimeTableTick2Location(self->timeTable, tick);

    _PlayerClockSourceNotifyClock(self, tick, prevTick, self->usec, location);

    if (TimeTableLength(self->timeTable) <= tick) {
        _PlayerClockSourceNotifyEvent(self, PlayerClockSourceEventReachEnd);
    }
}

static void _PlayerClockSourceSetTimeTable(PlayerClockSource *self, void *data)
{
    if (self->timeTable) {
        TimeTableRelease(self->timeTable);
    }
    self->timeTable = data;
}

static void _PlayerClockSourceStop(PlayerClockSource *self, void *data)
{
    if (!self->playing) {
        return;
    }

    self->playing = false;
    _PlayerClockSourceNotifyEvent(self, PlayerClockSourceEventStop);
}

static void _PlayerClockSourcePlay(PlayerClockSource *self, void *data)
{
    if (self->playing) {
        return;
    }

    self->start = currentMicroSec();
    self->offset = self->usec;

    self->playing = true;
    _PlayerClockSourceNotifyEvent(self, PlayerClockSourceEventPlay);
}

static void _PlayerClockSourceRewind(PlayerClockSource *self, void *data)
{
    self->offset = 0;
    self->usec = 0;
    self->start = currentMicroSec();

    _PlayerClockSourceNotifyClock(self, 0, 0, 0, LocationZero);
    _PlayerClockSourceNotifyEvent(self, PlayerClockSourceEventRewind);
}

static void _PlayerClockSourceForward(PlayerClockSource *self, void *data)
{
    int32_t tick = TimeTableMicroSec2Tick(self->timeTable, self->usec);
    Location location = TimeTableTick2Location(self->timeTable, tick);
    location.m++;
    location.b = 1;
    location.t = 0;

    tick = TimeTableTickByMeasure(self->timeTable, location.m);
    tick = MIN(tick, TimeTableLength(self->timeTable));

    self->usec = TimeTableTick2MicroSec(self->timeTable, tick);
    self->offset = self->usec;
    self->start = currentMicroSec();

    _PlayerClockSourceNotifyClock(self, tick, tick, self->usec, location);
    _PlayerClockSourceNotifyEvent(self, PlayerClockSourceEventForward);
}

static void _PlayerClockSourceBackward(PlayerClockSource *self, void *data)
{
    int32_t tick = TimeTableMicroSec2Tick(self->timeTable, self->usec);
    Location location = TimeTableTick2Location(self->timeTable, tick);
    location.m -= 1 == location.b ? 1 : 0;
    location.m = MAX(1, location.m);
    location.b = 1;
    location.t = 0;

    tick = TimeTableTickByMeasure(self->timeTable, location.m);

    self->usec = TimeTableTick2MicroSec(self->timeTable, tick);
    self->offset = self->usec;
    self->start = currentMicroSec();

    _PlayerClockSourceNotifyClock(self, tick, tick, self->usec, location);
    _PlayerClockSourceNotifyEvent(self, PlayerClockSourceEventBackward);
}

static void _PlayerClockSourceDestroy(PlayerClockSource *self, void *data)
{
    if (self->timeTable) {
        TimeTableRelease(self->timeTable);
    }

    self->playing = false;
    self->exit = true;
}

static void __attribute__((constructor)) initializeTable()
{
    functionTable[PlayerMessageSetTimeTable] = _PlayerClockSourceSetTimeTable;
    functionTable[PlayerMessageStop] = _PlayerClockSourceStop;
    functionTable[PlayerMessagePlay] = _PlayerClockSourcePlay;
    functionTable[PlayerMessageRewind] = _PlayerClockSourceRewind;
    functionTable[PlayerMessageForward] = _PlayerClockSourceForward;
    functionTable[PlayerMessageBackward] = _PlayerClockSourceBackward;
    functionTable[PlayerMessageDestroy] = _PlayerClockSourceDestroy;
}
