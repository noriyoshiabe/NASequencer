#pragma once

#include <stdbool.h>

typedef struct _NAMessage {
    int kind;
    void *data;
} NAMessage;

typedef struct _NAMessageQ NAMessageQ;

extern NAMessageQ *NAMessageQCreate();
extern void NAMessageQDestroy(NAMessageQ *self);
extern bool NAMessageQPost(NAMessageQ *self, int kind, void *data);
extern bool NAMessageQWait(NAMessageQ *self, NAMessage *message);
extern bool NAMessageQPeek(NAMessageQ *self, NAMessage *message);
