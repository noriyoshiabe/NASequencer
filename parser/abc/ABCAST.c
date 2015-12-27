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

static void ASTFileIdentificationAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitFileIdentification(visitor, self);
}

static void ASTFileIdentificationDestroy(void *_self)
{
    ASTFileIdentification *self = _self;
    free(self->identifier);
}

ASTFileIdentification *ABCASTFileIdentificationCreate(FileLocation *location)
{
    return NodeCreate(ASTFileIdentification, location);
}
