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

static void ASTIncludeAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitInclude(visitor, self);
}

static void ASTIncludeDestroy(void *_self)
{
    ASTInclude *self = _self;

    free(self->filepath);
    free(self->fullpath);

    if (self->root) {
        NodeRelease(self->root);
    }
}

ASTInclude *MMLASTIncludeCreate(FileLocation *location)
{
    return NodeCreate(ASTInclude, location);
}
