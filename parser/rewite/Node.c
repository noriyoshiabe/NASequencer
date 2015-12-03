#include "Node.h"

#include <stdlib.h>

void *NodeCreate(int size, const char *type, FileLocation *location)
{
    Node *self = calloc(1, size);
    self->type = type;

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
