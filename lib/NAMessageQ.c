#include "NAMessageQ.h"

#include <stdlib.h>
#include <pthread.h>

#define MSGQ_SIZE 8
#define INC(idx) (++idx, idx &= ~MSGQ_SIZE)

struct _NAMessageQ {
    NAMessage msgQ[MSGQ_SIZE];
    int count;
    int readIdx;
    int writeIdx;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

NAMessageQ *NAMessageQCreate()
{
    NAMessageQ *self = calloc(1, sizeof(NAMessageQ));
    pthread_mutex_init(&self->mutex, NULL);
    pthread_cond_init(&self->cond, NULL);
    return self;
}

void NAMessageQDestroy(NAMessageQ *self)
{
    pthread_mutex_destroy(&self->mutex);
    pthread_cond_destroy(&self->cond);
    free(self);
}

bool NAMessageQPost(NAMessageQ *self, int kind, void *data)
{
    pthread_mutex_lock(&self->mutex);

    if (MSGQ_SIZE <= self->count) {
        pthread_mutex_unlock(&self->mutex);
        return false;
    }

    self->msgQ[self->writeIdx].kind = kind;
    self->msgQ[self->writeIdx].data = data;
    INC(self->writeIdx);
    ++self->count;
    pthread_cond_signal(&self->cond);

    pthread_mutex_unlock(&self->mutex);

    return true;
}

bool NAMessageQWait(NAMessageQ *self, NAMessage *message)
{
    pthread_mutex_lock(&self->mutex);

    if (0 == self->count) {
        pthread_cond_wait(&self->cond, &self->mutex);
    }

    *message = self->msgQ[self->readIdx];
    INC(self->readIdx);
    --self->count;

    pthread_mutex_unlock(&self->mutex);

    return true;
}

bool NAMessageQPeek(NAMessageQ *self, NAMessage *message)
{
    pthread_mutex_lock(&self->mutex);

    if (0 == self->count) {
        pthread_mutex_unlock(&self->mutex);
        return false;
    }

    *message = self->msgQ[self->readIdx];
    INC(self->readIdx);
    --self->count;

    pthread_mutex_unlock(&self->mutex);

    return true;
}
