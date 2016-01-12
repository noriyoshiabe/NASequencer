#include "MMLAST.h"

#include <stdlib.h>

static void ASTRootAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitRoot(visitor, self);
}

static void ASTRootDestroy(void *_self)
{
}

ASTRoot *MMLASTRootCreate(FileLocation *location)
{
    return NodeCreate(ASTRoot, location);
}
