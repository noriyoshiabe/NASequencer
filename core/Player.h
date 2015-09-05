#pragma once

#include "Mixer.h"

typedef struct _Player Player;

typedef struct _PlayerCallbacks {
} PlayerCallbacks;

extern Player *PlayerCreate(Mixer *mixer);
extern void PlayerDestroy(Player *self);
extern void PlayerAddObserver(Player *self, void *receiver, PlayerCallbacks *callbacks);
extern void PlayerRemoveObserver(Player *self, void *receiver);
