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
    PlayerEventSeek,
    PlayerEventReachEnd,
    PlayerEventTempoChange,
    PlayerEventTimeSignChange,
    PlayerEventRepeatStateChange,
} PlayerEvent;

typedef enum {
    PlayerRepeatStateOff,
    PlayerRepeatStateRepeatAll,
    PlayerRepeatStateRepeatSection,
} PlayerRepeatState;

typedef struct _Player Player;

typedef struct _PlayerObserverCallbacks {
    void (*onNotifyClock)(void *receiver, int tick, int64_t usec, Location location);
    void (*onNotifyEvent)(void *receiver, PlayerEvent event);
    void (*onSendNoteOn)(void *receiver, NoteEvent *event);
    void (*onSendNoteOff)(void *receiver, NoteEvent *event);
} PlayerObserverCallbacks;

extern Player *PlayerCreate(Mixer *mixer);
extern void PlayerDestroy(Player *self);
extern void PlayerAddObserver(Player *self, void *receiver, PlayerObserverCallbacks *callbacks);
extern void PlayerRemoveObserver(Player *self, void *receiver);

extern void PlayerSetSequence(Player *self, Sequence *sequence);
extern void PlayerPlay(Player *self);
extern void PlayerStop(Player *self);
extern void PlayerPlayPause(Player *self);
extern void PlayerRewind(Player *self);
extern void PlayerForward(Player *self);
extern void PlayerBackWard(Player *self);
extern void PlayerSeek(Player *self, Location location);
extern void PlayerToggleRepeat(Player *self);

extern bool PlayerIsPlaying(Player *self);
extern uint64_t PlayerGetUsec(Player *self);
extern int PlayerGetTick(Player *self);
extern Location PlayerGetLocation(Player *self);
extern float PlayerGetTempo(Player *self);
extern TimeSign PlayerGetTimeSign(Player *self);
extern PlayerRepeatState PlayerGetRepeatState(Player *self);

static inline const char *PlayerEvent2String(PlayerEvent event)
{
#define CASE(event) case event: return &(#event[11])
    switch (event) {
    CASE(PlayerEventStop);
    CASE(PlayerEventPlay);
    CASE(PlayerEventRewind);
    CASE(PlayerEventForward);
    CASE(PlayerEventBackward);
    CASE(PlayerEventSeek);
    CASE(PlayerEventReachEnd);
    CASE(PlayerEventTempoChange);
    CASE(PlayerEventTimeSignChange);
    CASE(PlayerEventRepeatStateChange);
    }
    return "Unknown player event";
#undef CASE
}
