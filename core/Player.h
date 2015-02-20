#pragma once

#include <NAType.h>
#include "Sequence.h"

typedef enum _PlayerState {
    PLAYER_STATE_INVALID,
    PLAYER_STATE_STOP,
    PLAYER_STATE_PLAYING,
    PLAYER_STATE_EXIT,
} PlayerState;

typedef struct _PlayerContext {
    int state;
    uint64_t usec;
    Location location;
    CFMutableArrayRef playing;
    float tempo;
    uint8_t numerator;
    uint8_t denominator;
} PlayerContext;

typedef struct _Player Player;
NAExportClass(Player);

typedef struct _PlayerObserverVtbl {
    void (*onPlayerContextChanged)(void *self, Player *sender, PlayerContext *context);
} PlayerObserverVtbl;

NAExportClass(PlayerObserver);

extern void PlayerAddObserver(Player *self, void *observer);
extern void PlayerSetSource(Player *self, void *source);
extern void PlayerPlay(Player *self);
extern void PlayerStop(Player *self);
extern void PlayerRewind(Player *self);
extern void PlayerForward(Player *self);
extern void PlayerBackward(Player *self);
extern bool PlayerIsPlaying(Player *self);

extern const char *PlayerState2String(int state);
extern CFStringRef PlayerState2CFString(int state);
