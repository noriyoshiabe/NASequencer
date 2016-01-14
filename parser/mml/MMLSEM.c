#include "MMLSEM.h"

#include <stdlib.h>

static void SEMListAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitList(visitor, self);
}

static void SEMListDestroy(void *_self)
{
    SEMList *self = _self;
}

SEMList *MMLSEMListCreate(FileLocation *location)
{
    SEMList *self = NodeCreate(SEMList, location);
    self->node.children = NAArrayCreate(4, NULL);
    return self;
}
