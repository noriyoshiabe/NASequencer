#pragma once

#include "NAArray.h"

typedef struct _Node {
    const char *type;
    struct {
        const char *filepath;
        int line;
        int column;
    } location;

    void (*accept)(void *self, void *visitor);
    void (*destroy)(void *self);

    NAArray *children;
} Node;

extern void *NodeCreate(int size, const char *type, const char *filepath, int line, int column,
        void (*accept)(void *, void *), void (*destroy)(void *));
extern void NodeDestroy(void *self);
extern void NodeDump(void *self, int indent);
