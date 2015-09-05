#pragma once

#include "Mixer.h"
#include "Sequence.h"

#include <stdint.h>

typedef enum {
    PlayerEventStop,
    PlayerEventPlay,
    PlayerEventRewind,
    PlayerEventForward,
    PlayerEventBackward,
    PlayerEventReachEnd,
} PlayerEvent;

typedef struct _Player Player;

typedef struct _PlayerCallbacks {
    void (*onNotifyClock)(void *receiver, int tick, int prevTick, int64_t usec, Location location);
    void (*onNotifyEvent)(void *receiver, PlayerEvent event);
    void (*onSendNoteOn)(void *receiver, NoteEvent *event);
    void (*onSendNoteOff)(void *receiver, NoteEvent *event);
} PlayerCallbacks;

extern Player *PlayerCreate(Mixer *mixer);
extern void PlayerDestroy(Player *self);
extern void PlayerAddObserver(Player *self, void *receiver, PlayerCallbacks *callbacks);
extern void PlayerRemoveObserver(Player *self, void *receiver);

extern void PlayerSetSequence(Player *self, Sequence *sequence);
extern void PlayerPlay(Player *self);
extern void PlayerStop(Player *self);
extern void PlayerPlayPause(Player *self);
extern void PlayerRewind(Player *self);
extern void PlayerForward(Player *self);
extern void PlayerBackWard(Player *self);

static inline const char *PlayerEvent2String(PlayerEvent event)
{
#define CASE(event) case event: return &(#event[11])
    switch (event) {
    CASE(PlayerEventStop);
    CASE(PlayerEventPlay);
    CASE(PlayerEventRewind);
    CASE(PlayerEventForward);
    CASE(PlayerEventBackward);
    CASE(PlayerEventReachEnd);
    }
    return "Unknown player event";
#undef CASE
}
