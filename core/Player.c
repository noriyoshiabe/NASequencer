#include "Player.h"
#include "NAMessageQ.h"
#include "NAMap.h"

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/param.h>

struct _Player {
    NAMap *observers;
    NAMessageQ *msgQ;
    pthread_t thread;
    bool exit;

    Sequence *sequence;

    bool playing;

    int64_t usec;
    int64_t start;
    int64_t offset;
};

Player *PlayerCreate(Mixer *mixer)
{
    Player *self = calloc(1, sizeof(Player));
    self->observers = NAMapCreate(NULL, NULL, NULL);
    return self;
}

void PlayerDestroy(Player *self)
{
    NAMapDestroy(self->observers);
    free(self);
}

void PlayerAddObserver(Player *self, void *receiver, PlayerCallbacks *callbacks)
{
    NAMapPut(self->observers, receiver, callbacks);
}

void PlayerRemoveObserver(Player *self, void *receiver)
{
    NAMapRemove(self->observers, receiver);
}

void PlayerSetSequence(Player *self, Sequence *sequence)
{
}

void PlayerPlay(Player *self)
{
}

void PlayerStop(Player *self)
{
}

void PlayerPlayPause(Player *self)
{
}

void PlayerRewind(Player *self)
{
}

void PlayerForward(Player *self)
{
}

void PlayerBackWard(Player *self)
{
}
