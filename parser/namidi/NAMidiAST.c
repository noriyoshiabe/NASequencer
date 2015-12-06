#include "NAMidiAST.h"

#include <stdlib.h>

static void ASTRootAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitRoot(visitor, self);
}

static void ASTRootDestroy(void *_self)
{
}

ASTRoot *NAMidiASTRootCreate(FileLocation *location)
{
    return NodeCreate(ASTRoot, location);
}

static void ASTResolutionAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitResolution(visitor, self);
}

static void ASTResolutionDestroy(void *_self)
{
}

ASTResolution *NAMidiASTResolutionCreate(FileLocation *location)
{
    return NodeCreate(ASTResolution, location);
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

ASTTitle *NAMidiASTTitleCreate(FileLocation *location)
{
    return NodeCreate(ASTTitle, location);
}

static void ASTTempoAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTempo(visitor, self);
}

static void ASTTempoDestroy(void *_self)
{
}

ASTTempo *NAMidiASTTempoCreate(FileLocation *location)
{
    return NodeCreate(ASTTempo, location);
}

static void ASTTimeAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTime(visitor, self);
}

static void ASTTimeDestroy(void *_self)
{
}

ASTTime *NAMidiASTTimeCreate(FileLocation *location)
{
    return NodeCreate(ASTTime, location);
}

static void ASTKeyAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitKey(visitor, self);
}

static void ASTKeyDestroy(void *_self)
{
    ASTKey *self = _self;
    free(self->keyString);
}

ASTKey *NAMidiASTKeyCreate(FileLocation *location)
{
    return NodeCreate(ASTKey, location);
}

static void ASTMarkerAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitMarker(visitor, self);
}

static void ASTMarkerDestroy(void *_self)
{
    ASTMarker *self = _self;
    free(self->text);
}

ASTMarker *NAMidiASTMarkerCreate(FileLocation *location)
{
    return NodeCreate(ASTMarker, location);
}

static void ASTChannelAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitChannel(visitor, self);
}

static void ASTChannelDestroy(void *_self)
{
}

ASTChannel *NAMidiASTChannelCreate(FileLocation *location)
{
    return NodeCreate(ASTChannel, location);
}

static void ASTVoiceAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitVoice(visitor, self);
}

static void ASTVoiceDestroy(void *_self)
{
}

ASTVoice *NAMidiASTVoiceCreate(FileLocation *location)
{
    return NodeCreate(ASTVoice, location);
}

static void ASTSynthAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitSynth(visitor, self);
}

static void ASTSynthDestroy(void *_self)
{
    ASTSynth *self = _self;
    free(self->name);
}

ASTSynth *NAMidiASTSynthCreate(FileLocation *location)
{
    return NodeCreate(ASTSynth, location);
}

static void ASTVolumeAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitVolume(visitor, self);
}

static void ASTVolumeDestroy(void *_self)
{
}

ASTVolume *NAMidiASTVolumeCreate(FileLocation *location)
{
    return NodeCreate(ASTVolume, location);
}

static void ASTPanAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitPan(visitor, self);
}

static void ASTPanDestroy(void *_self)
{
}

ASTPan *NAMidiASTPanCreate(FileLocation *location)
{
    return NodeCreate(ASTPan, location);
}

static void ASTChorusAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitChorus(visitor, self);
}

static void ASTChorusDestroy(void *_self)
{
}

ASTChorus *NAMidiASTChorusCreate(FileLocation *location)
{
    return NodeCreate(ASTChorus, location);
}

static void ASTReverbAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitReverb(visitor, self);
}

static void ASTReverbDestroy(void *_self)
{
}

ASTReverb *NAMidiASTReverbCreate(FileLocation *location)
{
    return NodeCreate(ASTReverb, location);
}

static void ASTTransposeAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTranspose(visitor, self);
}

static void ASTTransposeDestroy(void *_self)
{
}

ASTTranspose *NAMidiASTTransposeCreate(FileLocation *location)
{
    return NodeCreate(ASTTranspose, location);
}

static void ASTRestAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitRest(visitor, self);
}

static void ASTRestDestroy(void *_self)
{
}

ASTRest *NAMidiASTRestCreate(FileLocation *location)
{
    return NodeCreate(ASTRest, location);
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

ASTNote *NAMidiASTNoteCreate(FileLocation *location)
{
    return NodeCreate(ASTNote, location);
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

ASTInclude *NAMidiASTIncludeCreate(FileLocation *location)
{
    return NodeCreate(ASTInclude, location);
}

static void ASTPatternAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitPattern(visitor, self);
}

static void ASTPatternDestroy(void *_self)
{
    ASTPattern *self = _self;
    free(self->identifier);
}

ASTPattern *NAMidiASTPatternCreate(FileLocation *location)
{
    return NodeCreate(ASTPattern, location);
}

static void ASTDefineAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitDefine(visitor, self);
}

static void ASTDefineDestroy(void *_self)
{
    ASTDefine *self = _self;
    free(self->identifier);
}

ASTDefine *NAMidiASTDefineCreate(FileLocation *location)
{
    return NodeCreate(ASTDefine, location);
}

static void ASTContextAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitContext(visitor, self);
}

static void ASTContextDestroy(void *_self)
{
    ASTContext *self = _self;
    NAArrayTraverse(self->ctxIdList, NodeRelease);
    NAArrayDestroy(self->ctxIdList);
}

ASTContext *NAMidiASTContextCreate(FileLocation *location)
{
    return NodeCreate(ASTContext, location);
}

static void ASTIdentifierAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitIdentifier(visitor, self);
}

static void ASTIdentifierDestroy(void *_self)
{
    ASTIdentifier *self = _self;
    free(self->idString);
}

ASTIdentifier *NAMidiASTIdentifierCreate(FileLocation *location)
{
    return NodeCreate(ASTIdentifier, location);
}

static void ASTNoteParamAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitNoteParam(visitor, self);
}

static void ASTNoteParamDestroy(void *_self)
{
}

ASTNoteParam *NAMidiASTNoteParamCreate(FileLocation *location)
{
    return NodeCreate(ASTNoteParam, location);
}
