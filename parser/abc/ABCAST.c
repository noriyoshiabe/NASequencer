#include "ABCAST.h"

#include <stdlib.h>

static void ASTRootAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitRoot(visitor, self);
}

static void ASTRootDestroy(void *_self)
{
}

ASTRoot *ABCASTRootCreate(FileLocation *location)
{
    return NodeCreate(ASTRoot, location);
}
