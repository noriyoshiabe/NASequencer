#pragma once

#include "NAArray.h"

typedef struct _Node {
    const char *type;
    void (*accept)(void *self, void *visitor);
    void (*destroy)(void *self);
    NAArray *children;
} Node;

extern void *NodeCreate(int size, const char *type, void (*accept)(void *, void *), void (*destroy)(void *));
extern void NodeDestroy(void *self);
