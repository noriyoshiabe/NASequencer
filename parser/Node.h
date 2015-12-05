#pragma once

#include "FileLocation.h"
#include "NAArray.h"

typedef struct _Node {
    const char *type;
    FileLocation location;
    NAArray *children;

    void (*accept)(void *self, void *visitor);
    void (*destroy)(void *self);

    int refCount;
} Node;

extern void *NodeCreate(int size, const char *type, FileLocation *location,
        void (*accept)(void *self, void *visitor), void (*destroy)(void *self));
extern void *NodeRetain(void *self);
extern void NodeRelease(void *self);

#define NodeCreate(type, location) \
    NodeCreate(sizeof(type), #type, location, type##Accept, type##Destroy)