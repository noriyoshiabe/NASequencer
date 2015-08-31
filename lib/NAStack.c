#include "NAStack.h"

#include <stdlib.h>
#include <stdint.h>

struct _NAStack {
    void **stack;
    int size;
    int pointer;
};

NAStack *NAStackCreate(int initialSize)
{
    NAStack *self = calloc(1, sizeof(NAStack));
    self->size = initialSize;
    self->stack = malloc(initialSize * sizeof(void *));
    self->pointer = -1;
    return self;
}

void NAStackDestroy(NAStack *self)
{
    free(self->stack);
    free(self);
}

void NAStackPush(NAStack *self, void *value)
{
    if (self->size <= self->pointer + 1) {
        self->size *= 2;
        self->stack = realloc(self->stack, self->size * sizeof(void *));
    }

    self->stack[++self->pointer] = value;
}

void *NAStackPop(NAStack *self)
{
    if (0 > self->pointer) {
        return NULL;
    }

    return self->stack[self->pointer--];
}
