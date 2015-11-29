#pragma once

#include "NAArray.h"

typedef struct _FileLocation {
    const char *filepath;
    int line;
    int column;
} FileLocation;

typedef struct _Node {
    const char *type;
    FileLocation location;

    void (*accept)(void *self, void *visitor);
    void (*destroy)(void *self);

    NAArray *children;
} Node;

extern void *NodeCreate(int size, const char *type, FileLocation *location,
        void (*accept)(void *, void *), void (*destroy)(void *));
extern void NodeDestroy(void *self);
extern void NodeDump(void *self, int indent);
