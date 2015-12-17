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
    if (self->versionString) {
        free(self->versionString);
    }
    if (self->numberString) {
        free(self->numberString);
    }
}

ASTVersion *ABCASTVersionCreate(FileLocation *location)
{
    return NodeCreate(ASTVersion, location);
}

static void ASTStringInformationAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitStringInformation(visitor, self);
}

static void ASTStringInformationDestroy(void *_self)
{
    ASTStringInformation *self = _self;
    free(self->string);
}

ASTStringInformation *ABCASTStringInformationCreate(FileLocation *location)
{
    return NodeCreate(ASTStringInformation, location);
}

static void ASTReferenceNumberAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitReferenceNumber(visitor, self);
}

static void ASTReferenceNumberDestroy(void *_self)
{
}

ASTReferenceNumber *ABCASTReferenceNumberCreate(FileLocation *location)
{
    return NodeCreate(ASTReferenceNumber, location);
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

static void ASTKeyAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitKey(visitor, self);
}

static void ASTKeyDestroy(void *_self)
{
}

ASTKey *ABCASTKeyCreate(FileLocation *location)
{
    return NodeCreate(ASTKey, location);
}

static void ASTKeyParamAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitKeyParam(visitor, self);
}

static void ASTKeyParamDestroy(void *_self)
{
    ASTKeyParam *self = _self;
    if (self->string) {
        free(self->string);
    }
}

ASTKeyParam *ABCASTKeyParamCreate(FileLocation *location)
{
    return NodeCreate(ASTKeyParam, location);
}

static void ASTMeterAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitMeter(visitor, self);
}

static void ASTMeterDestroy(void *_self)
{
}

ASTMeter *ABCASTMeterCreate(FileLocation *location)
{
    return NodeCreate(ASTMeter, location);
}

static void ASTUnitNoteLengthAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitUnitNoteLength(visitor, self);
}

static void ASTUnitNoteLengthDestroy(void *_self)
{
}

ASTUnitNoteLength *ABCASTUnitNoteLengthCreate(FileLocation *location)
{
    return NodeCreate(ASTUnitNoteLength, location);
}

static void ASTTempoAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTempo(visitor, self);
}

static void ASTTempoDestroy(void *_self)
{
}

ASTTempo *ABCASTTempoCreate(FileLocation *location)
{
    return NodeCreate(ASTTempo, location);
}

static void ASTTempoParamAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTempoParam(visitor, self);
}

static void ASTTempoParamDestroy(void *_self)
{
    ASTTempoParam *self = _self;
    if (self->string) {
        free(self->string);
    }
}

ASTTempoParam *ABCASTTempoParamCreate(FileLocation *location)
{
    return NodeCreate(ASTTempoParam, location);
}

static void ASTPartsAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitParts(visitor, self);
}

static void ASTPartsDestroy(void *_self)
{
    ASTParts *self = _self;
    free(self->list);
}

ASTParts *ABCASTPartsCreate(FileLocation *location)
{
    return NodeCreate(ASTParts, location);
}

static void ASTInstructionAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitInstruction(visitor, self);
}

static void ASTInstructionDestroy(void *_self)
{
    ASTInstruction *self = _self;
    free(self->string);
}

ASTInstruction *ABCASTInstructionCreate(FileLocation *location)
{
    return NodeCreate(ASTInstruction, location);
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
