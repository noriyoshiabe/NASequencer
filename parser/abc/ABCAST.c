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

static void ASTInstCharSetAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitInstCharSet(visitor, self);
}

static void ASTInstCharSetDestroy(void *_self)
{
    ASTInstCharSet *self = _self;
    free(self->name);
}

ASTInstCharSet *ABCASTInstCharSetCreate(FileLocation *location)
{
    return NodeCreate(ASTInstCharSet, location);
}

static void ASTInstVersionAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitInstVersion(visitor, self);
}

static void ASTInstVersionDestroy(void *_self)
{
    ASTInstVersion *self = _self;
    free(self->numberString);
}

ASTInstVersion *ABCASTInstVersionCreate(FileLocation *location)
{
    return NodeCreate(ASTInstVersion, location);
}

static void ASTInstIncludeAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitInstInclude(visitor, self);
}

static void ASTInstIncludeDestroy(void *_self)
{
    ASTInstInclude *self = _self;

    free(self->filepath);

    if (self->fullpath) {
        free(self->fullpath);
    }

    if (self->root) {
        NodeRelease(self->root);
    }
}

ASTInstInclude *ABCASTInstIncludeCreate(FileLocation *location)
{
    return NodeCreate(ASTInstInclude, location);
}

static void ASTInstCreatorAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitInstCreator(visitor, self);
}

static void ASTInstCreatorDestroy(void *_self)
{
    ASTInstCreator *self = _self;
    free(self->name);
}

ASTInstCreator *ABCASTInstCreatorCreate(FileLocation *location)
{
    return NodeCreate(ASTInstCreator, location);
}

static void ASTSymbolLineAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitSymbolLine(visitor, self);
}

static void ASTSymbolLineDestroy(void *_self)
{
    ASTSymbolLine *self = _self;
    free(self->string);
}

ASTSymbolLine *ABCASTSymbolLineCreate(FileLocation *location)
{
    return NodeCreate(ASTSymbolLine, location);
}

static void ASTContinuationAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitContinuation(visitor, self);
}

static void ASTContinuationDestroy(void *_self)
{
    ASTContinuation *self = _self;
    free(self->string);
}

ASTContinuation *ABCASTContinuationCreate(FileLocation *location)
{
    return NodeCreate(ASTContinuation, location);
}

static void ASTVoiceAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitVoice(visitor, self);
}

static void ASTVoiceDestroy(void *_self)
{
}

ASTVoice *ABCASTVoiceCreate(FileLocation *location)
{
    return NodeCreate(ASTVoice, location);
}

static void ASTVoiceParamAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitVoiceParam(visitor, self);
}

static void ASTVoiceParamDestroy(void *_self)
{
    ASTVoiceParam *self = _self;
    if (self->string) {
        free(self->string);
    }
}

ASTVoiceParam *ABCASTVoiceParamCreate(FileLocation *location)
{
    return NodeCreate(ASTVoiceParam, location);
}

static void ASTTuneBodyAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTuneBody(visitor, self);
}

static void ASTTuneBodyDestroy(void *_self)
{
}

ASTTuneBody *ABCASTTuneBodyCreate(FileLocation *location)
{
    return NodeCreate(ASTTuneBody, location);
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

static void ASTAnnotationAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitAnnotation(visitor, self);
}

static void ASTAnnotationDestroy(void *_self)
{
    ASTAnnotation *self = _self;
    free(self->text);
}

ASTAnnotation *ABCASTAnnotationCreate(FileLocation *location)
{
    return NodeCreate(ASTAnnotation, location);
}

static void ASTDecorationAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitDecoration(visitor, self);
}

static void ASTDecorationDestroy(void *_self)
{
    ASTDecoration *self = _self;
    free(self->symbol);
}

ASTDecoration *ABCASTDecorationCreate(FileLocation *location)
{
    return NodeCreate(ASTDecoration, location);
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
