#include "NAMidiAST.h"

#include <stdlib.h>

static void ASTRootAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitRoot(visitor, self);
}

static void ASTRootDestroy(void *self)
{
}

ASTRoot *ASTRootCreate(FileLocation *location)
{
    return NodeCreate(ASTRoot, location);
}

static void ASTResolutionAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitResolution(visitor, self);
}

static void ASTResolutionDestroy(void *self)
{
}

ASTResolution *ASTResolutionCreate(FileLocation *location)
{
    return NodeCreate(ASTResolution, location);
}

static void ASTTitleAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTitle(visitor, self);
}

static void ASTTitleDestroy(void *self)
{
}

ASTTitle *ASTTitleCreate(FileLocation *location)
{
    return NodeCreate(ASTTitle, location);
}

static void ASTTempoAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTempo(visitor, self);
}

static void ASTTempoDestroy(void *self)
{
}

ASTTempo *ASTTempoCreate(FileLocation *location)
{
    return NodeCreate(ASTTempo, location);
}

static void ASTTimeAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTime(visitor, self);
}

static void ASTTimeDestroy(void *self)
{
}

ASTTime *ASTTimeCreate(FileLocation *location)
{
    return NodeCreate(ASTTime, location);
}

static void ASTKeyAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitKey(visitor, self);
}

static void ASTKeyDestroy(void *self)
{
}

ASTKey *ASTKeyCreate(FileLocation *location)
{
    return NodeCreate(ASTKey, location);
}

static void ASTMarkerAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitMarker(visitor, self);
}

static void ASTMarkerDestroy(void *self)
{
}

ASTMarker *ASTMarkerCreate(FileLocation *location)
{
    return NodeCreate(ASTMarker, location);
}

static void ASTChannelAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitChannel(visitor, self);
}

static void ASTChannelDestroy(void *self)
{
}

ASTChannel *ASTChannelCreate(FileLocation *location)
{
    return NodeCreate(ASTChannel, location);
}

static void ASTVoiceAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitVoice(visitor, self);
}

static void ASTVoiceDestroy(void *self)
{
}

ASTVoice *ASTVoiceCreate(FileLocation *location)
{
    return NodeCreate(ASTVoice, location);
}

static void ASTSynthAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitSynth(visitor, self);
}

static void ASTSynthDestroy(void *self)
{
}

ASTSynth *ASTSynthCreate(FileLocation *location)
{
    return NodeCreate(ASTSynth, location);
}

static void ASTVolumeAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitVolume(visitor, self);
}

static void ASTVolumeDestroy(void *self)
{
}

ASTVolume *ASTVolumeCreate(FileLocation *location)
{
    return NodeCreate(ASTVolume, location);
}

static void ASTPanAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitPan(visitor, self);
}

static void ASTPanDestroy(void *self)
{
}

ASTPan *ASTPanCreate(FileLocation *location)
{
    return NodeCreate(ASTPan, location);
}

static void ASTChorusAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitChorus(visitor, self);
}

static void ASTChorusDestroy(void *self)
{
}

ASTChorus *ASTChorusCreate(FileLocation *location)
{
    return NodeCreate(ASTChorus, location);
}

static void ASTReverbAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitReverb(visitor, self);
}

static void ASTReverbDestroy(void *self)
{
}

ASTReverb *ASTReverbCreate(FileLocation *location)
{
    return NodeCreate(ASTReverb, location);
}

static void ASTTransposeAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTranspose(visitor, self);
}

static void ASTTransposeDestroy(void *self)
{
}

ASTTranspose *ASTTransposeCreate(FileLocation *location)
{
    return NodeCreate(ASTTranspose, location);
}

static void ASTRestAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitRest(visitor, self);
}

static void ASTRestDestroy(void *self)
{
}

ASTRest *ASTRestCreate(FileLocation *location)
{
    return NodeCreate(ASTRest, location);
}

static void ASTNoteAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitNote(visitor, self);
}

static void ASTNoteDestroy(void *self)
{
}

ASTNote *ASTNoteCreate(FileLocation *location)
{
    return NodeCreate(ASTNote, location);
}

static void ASTIncludeAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitInclude(visitor, self);
}

static void ASTIncludeDestroy(void *self)
{
}

ASTInclude *ASTIncludeCreate(FileLocation *location)
{
    return NodeCreate(ASTInclude, location);
}

static void ASTPatternAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitPattern(visitor, self);
}

static void ASTPatternDestroy(void *self)
{
}

ASTPattern *ASTPatternCreate(FileLocation *location)
{
    return NodeCreate(ASTPattern, location);
}

static void ASTDefineAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitDefine(visitor, self);
}

static void ASTDefineDestroy(void *self)
{
}

ASTDefine *ASTDefineCreate(FileLocation *location)
{
    return NodeCreate(ASTDefine, location);
}

static void ASTContextAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitContext(visitor, self);
}

static void ASTContextDestroy(void *self)
{
}

ASTContext *ASTContextCreate(FileLocation *location)
{
    return NodeCreate(ASTContext, location);
}

static void ASTIdentifierAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitIdentifier(visitor, self);
}

static void ASTIdentifierDestroy(void *self)
{
}

ASTIdentifier *ASTIdentifierCreate(FileLocation *location)
{
    return NodeCreate(ASTIdentifier, location);
}

static void ASTNoteParamAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitNoteParam(visitor, self);
}

static void ASTNoteParamDestroy(void *self)
{
}

ASTNoteParam *ASTNoteParamCreate(FileLocation *location)
{
    return NodeCreate(ASTNoteParam, location);
}
