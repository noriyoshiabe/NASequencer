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

static void ASTVersionAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitVersion(visitor, self);
}

static void ASTVersionDestroy(void *_self)
{
    ASTVersion *self = _self;
    free(self->versionString);
}

ASTVersion *ABCASTVersionCreate(FileLocation *location)
{
    return NodeCreate(ASTVersion, location);
}

static void ASTTitleAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTitle(visitor, self);
}

static void ASTTitleDestroy(void *_self)
{
    ASTTitle *self = _self;
    free(self->title);
}

ASTTitle *ABCASTTitleCreate(FileLocation *location)
{
    return NodeCreate(ASTTitle, location);
}

static void ASTNoteAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitNote(visitor, self);
}

static void ASTNoteDestroy(void *_self)
{
    ASTNote *self = _self;
    free(self->noteString);
}

ASTNote *ABCASTNoteCreate(FileLocation *location)
{
    return NodeCreate(ASTNote, location);
}

static void ASTLineBreakAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitLineBreak(visitor, self);
}

static void ASTLineBreakDestroy(void *_self)
{
}

ASTLineBreak *ABCASTLineBreakCreate(FileLocation *location)
{
    return NodeCreate(ASTLineBreak, location);
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

ASTInclude *ABCASTIncludeCreate(FileLocation *location)
{
    return NodeCreate(ASTInclude, location);
}
