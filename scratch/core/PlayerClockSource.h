#pragma once

#include "TimeTable.h"

typedef enum {
    PlayerClockSourceEventStop,
    PlayerClockSourceEventPlay,
    PlayerClockSourceEventRewind,
    PlayerClockSourceEventForward,
    PlayerClockSourceEventBackward,
    PlayerClockSourceEventReachEnd,
} PlayerClockSourceEvent;

typedef struct _PlayerClockSource PlayerClockSource;

typedef struct _PlayerClockSourceCallbacks {
    void (*onNotifyClock)(void *receiver, int32_t tick, int32_t prevTick, int64_t usec, Location location);
    void (*onNotifyEvent)(void *receiver, PlayerClockSourceEvent event);
} PlayerClockSourceCallbacks;

extern PlayerClockSource *PlayerClockSourceCreate(PlayerClockSourceCallbacks *callbacks, void *receiver);
extern void PlayerClockSourceDestroy(PlayerClockSource *self);
extern void PlayerClockSourceSetTimeTable(PlayerClockSource *self, TimeTable *timeTable);
extern void PlayerClockSourceStop(PlayerClockSource *self);
extern void PlayerClockSourcePlay(PlayerClockSource *self);
extern void PlayerClockSourceRewind(PlayerClockSource *self);
extern void PlayerClockSourceFoward(PlayerClockSource *self);
extern void PlayerClockSourceBackword(PlayerClockSource *self);

static inline const char *PlayerClockSourceEvent2String(PlayerClockSourceEvent event)
{
#define CASE(event) case event: return &(#event[17])
    switch (event) {
    CASE(PlayerClockSourceEventStop);
    CASE(PlayerClockSourceEventPlay);
    CASE(PlayerClockSourceEventRewind);
    CASE(PlayerClockSourceEventForward);
    CASE(PlayerClockSourceEventBackward);
    CASE(PlayerClockSourceEventReachEnd);
    }
    return "Unknown event type";
#undef CASE
}
