#include "Node.h"

#include <stdlib.h>

void *NodeCreate(int size, const char *type, FileLocation *location,
        void (*accept)(void *, void *), void (*destroy)(void *), void (*dump)(void *, int))
{
    Node *self = calloc(1, size);
    self->type = type;
    if (location) {
        self->location = *location;
    }

    self->accept = accept;
    self->destroy = destroy;
    self->dump = dump;
    return self;
}

void NodeDestroy(void *_self)
{
    Node *self = _self;
    if (self->children) {
        NAIterator *iterator = NAArrayGetIterator(self->children);
        while (iterator->hasNext(iterator)) {
            NodeDestroy(iterator->next(iterator));
        }
    }

    free(self);
}

void NodeDump(void *_self, int indent)
{
    Node *self = _self;
    printf("%*s", indent, "");
    printf("[%s] %s:%d:%d\n", self->type, self->location.filepath, self->location.line, self->location.column);

    if (self->dump) {
        self->dump(self, indent);
    }

    if (self->children) {
        NAIterator *iterator = NAArrayGetIterator(self->children);
        while (iterator->hasNext(iterator)) {
            NodeDump(iterator->next(iterator), indent + 4);
        }
    }
}
