#pragma once

#include <NAType.h>
#include "Sequence.h"

typedef struct _Player Player;
NAExportClass(Player);

extern void PlayerSetSource(Player *self, void *source);
extern void PlayerPlay(Player *self);
extern void PlayerStop(Player *self);
extern void PlayerRewind(Player *self);
extern void PlayerForward(Player *self);
extern void PlayerBackward(Player *self);
extern bool PlayerIsPlaying(Player *self);
