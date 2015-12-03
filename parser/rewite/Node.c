#include "Node.h"

#include <stdlib.h>

#undef NodeCreate

void *NodeCreate(int size, const char *type, FileLocation *location,
        void (*accept)(void *self, void *visitor), void (*destroy)(void *self))
{
    Node *self = calloc(1, size);
    self->type = type;
    self->accept = accept;
    self->destroy = destroy;

    if (location) {
        self->location = *location;
    }

    self->refCount = 1;

    return self;
}

void *NodeRetain(void *_self)
{
    Node *self = _self;
    ++self->refCount;
    return self;
}

void NodeRelease(void *_self)
{
    Node *self = _self;
    if (0 == --self->refCount) {
        if (self->destroy) {
            self->destroy(self);
        }

        if (self->children) {
            NAArrayTraverse(self->children, NodeRelease);
            NAArrayDestroy(self->children);
        }

        free(self);
    }
}
