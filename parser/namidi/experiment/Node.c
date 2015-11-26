#include "Node.h"

#include <stdlib.h>

void *NodeCreate(int size, const char *type, const char *filepath, int line, int column,
        void (*accept)(void *, void *), void (*destroy)(void *))
{
    Node *self = calloc(1, size);
    self->type = type;
    self->location.filepath = filepath;
    self->location.line = line;
    self->location.column = column;
    self->accept = accept;
    self->destroy = destroy;
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

    if (self->children) {
        NAIterator *iterator = NAArrayGetIterator(self->children);
        while (iterator->hasNext(iterator)) {
            NodeDump(iterator->next(iterator), indent + 4);
        }
    }
}
