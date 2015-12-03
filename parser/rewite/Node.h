#pragma once

#include "FileLocation.h"

typedef struct _Node {
    const char *type;
    FileLocation location;
    NAArray *children;

    void (*accept)(void *self, void *visitor);
    void (*destroy)(void *self);

    int refCount;
} Node;

extern Node *NodeRetain(Node *node);
extern void NodeRelease(Node *node);