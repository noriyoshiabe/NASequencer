#include "Player.h"
#include "NAMessageQ.h"
#include "NAArray.h"

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/param.h>

typedef enum _PlayerMessage {
    PlayerMessageSetSequence,
    PlayerMessageStop,
    PlayerMessagePlay,
    PlayerMessageRewind,
    PlayerMessageForward,
    PlayerMessageBackward,
    PlayerMessageDestroy,
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

static void *PlayerRun(void *self);

Player *PlayerCreate(Mixer *mixer)
{
    Player *self = calloc(1, sizeof(Player));
    self->observers = NAArrayCreate(4, NULL);
    self->msgQ = NAMessageQCreate();
    pthread_create(&self->thread, NULL, PlayerRun, self);
    return self;
}

void PlayerDestroy(Player *self)
{
    self->playing = false;
    self->exit = true;
    NAMessageQPost(self->msgQ, PlayerMessageDestroy, NULL);
    pthread_join(self->thread, NULL);

    if (self->sequence) {
        SequenceRelease(self->sequence);
    }

    NAArrayTraverse(self->observers, free);
    NAArrayDestroy(self->observers);
    NAMessageQDestroy(self->msgQ);

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
    NAMessageQPost(self->msgQ, PlayerMessageSetSequence, SequenceRetain(sequence));
}

void PlayerPlay(Player *self)
{
    NAMessageQPost(self->msgQ, PlayerMessagePlay, NULL);
}

void PlayerStop(Player *self)
{
    NAMessageQPost(self->msgQ, PlayerMessageStop, NULL);
}

void PlayerPlayPause(Player *self)
{
    NAMessageQPost(self->msgQ, self->playing ? PlayerMessageStop : PlayerMessagePlay, NULL);
}

void PlayerRewind(Player *self)
{
    NAMessageQPost(self->msgQ, PlayerMessageRewind, NULL);
}

void PlayerForward(Player *self)
{
    NAMessageQPost(self->msgQ, PlayerMessageForward, NULL);
}

void PlayerBackWard(Player *self)
{
    NAMessageQPost(self->msgQ, PlayerMessageBackward, NULL);
}

static void PlayerProcessMessage(Player *self, PlayerMessage message, void *data)
{
}

static void PlayerSupplyClock(Player *self)
{
}

static void *PlayerRun(void *_self)
{
    Player *self = _self;

    while (!self->exit) {
        bool (*recvMessage)(NAMessageQ *, NAMessage *) = self->playing ? NAMessageQPeek : NAMessageQWait;

        NAMessage msg;
        if (recvMessage(self->msgQ, &msg)) {
            PlayerProcessMessage(self, msg.kind, msg.data);
        }

        if (self->playing) {
            PlayerSupplyClock(self);
            usleep(100);
        }
    }

    return NULL;
}
