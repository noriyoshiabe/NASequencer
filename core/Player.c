#include "Player.h"
#include "NAMessageQ.h"
#include "NAArray.h"

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

typedef struct Observer {
    void *receiver;
    PlayerObserverCallbacks *callbacks;
} Observer;

struct _Player {
    NAArray *observers;
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
    self->observers = NAArrayCreate(4, NULL);
    self->msgQ = NAMessageQCreate();
    pthread_create(&self->thread, NULL, _PlayerRun, self);
    return self;
}

void PlayerDestroy(Player *self)
{
    NAArrayTraverse(self->observers, free);
    NAArrayDestroy(self->observers);
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
