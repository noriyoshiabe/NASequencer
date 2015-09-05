#include "Player.h"

#include <stdlib.h>

struct _Player {
};

Player *PlayerCreate(Mixer *mixer)
{
    return calloc(1, sizeof(Player));
}

void PlayerDestroy(Player *self)
{
    free(self);
}
