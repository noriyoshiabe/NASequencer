#pragma once

#include "Node.h"

typedef struct _Statement {
    Node node;
    const char *type;
    void (*accept)(void *self, void *visitor);
    void (*destroy)(void *self);
} Statement;

extern void *StatementCreate(int size, const char *type, void (*accept)(void *, void *), void (*destroy)(void *));
extern void StatementDestroy(void *self);
