#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct _Message {
    int kind;
    void *arg;
} Message;

typedef struct _MessageQueue MessageQueue;

extern MessageQueue *MessageQueueCreate();
extern void MessageQueueDestroy(MessageQueue *self);
extern bool MessageQueuePost(MessageQueue *self, const Message *message);
extern bool MessageQueueWait(MessageQueue *self, Message *message);
extern bool MessageQueuePeek(MessageQueue *self, Message *message);
