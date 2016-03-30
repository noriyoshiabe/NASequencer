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

static void ASTCopyrightAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitCopyright(visitor, self);
}

static void ASTCopyrightDestroy(void *_self)
{
    ASTCopyright *self = _self;
    free(self->text);
}

ASTCopyright *NAMidiASTCopyrightCreate(FileLocation *location)
{
    return NodeCreate(ASTCopyright, location);
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

static void ASTPercussionAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitPercussion(visitor, self);
}

static void ASTPercussionDestroy(void *_self)
{
}

ASTPercussion *NAMidiASTPercussionCreate(FileLocation *location)
{
    return NodeCreate(ASTPercussion, location);
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

static void ASTVelocityAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitVelocity(visitor, self);
}

static void ASTVelocityDestroy(void *_self)
{
}

ASTVelocity *NAMidiASTVelocityCreate(FileLocation *location)
{
    return NodeCreate(ASTVelocity, location);
}

static void ASTGatetimeAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitGatetime(visitor, self);
}

static void ASTGatetimeDestroy(void *_self)
{
}

ASTGatetime *NAMidiASTGatetimeCreate(FileLocation *location)
{
    return NodeCreate(ASTGatetime, location);
}

static void ASTBankAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitBank(visitor, self);
}

static void ASTBankDestroy(void *_self)
{
}

ASTBank *NAMidiASTBankCreate(FileLocation *location)
{
    return NodeCreate(ASTBank, location);
}

static void ASTProgramAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitProgram(visitor, self);
}

static void ASTProgramDestroy(void *_self)
{
}

ASTProgram *NAMidiASTProgramCreate(FileLocation *location)
{
    return NodeCreate(ASTProgram, location);
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

static void ASTExpressionAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitExpression(visitor, self);
}

static void ASTExpressionDestroy(void *_self)
{
}

ASTExpression *NAMidiASTExpressionCreate(FileLocation *location)
{
    return NodeCreate(ASTExpression, location);
}

static void ASTPitchAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitPitch(visitor, self);
}

static void ASTPitchDestroy(void *_self)
{
}

ASTPitch *NAMidiASTPitchCreate(FileLocation *location)
{
    return NodeCreate(ASTPitch, location);
}

static void ASTDetuneAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitDetune(visitor, self);
}

static void ASTDetuneDestroy(void *_self)
{
}

ASTDetune *NAMidiASTDetuneCreate(FileLocation *location)
{
    return NodeCreate(ASTDetune, location);
}

static void ASTPitchSenseAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitPitchSense(visitor, self);
}

static void ASTPitchSenseDestroy(void *_self)
{
}

ASTPitchSense *NAMidiASTPitchSenseCreate(FileLocation *location)
{
    return NodeCreate(ASTPitchSense, location);
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

static void ASTStepAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitStep(visitor, self);
}

static void ASTStepDestroy(void *_self)
{
}

ASTStep *NAMidiASTStepCreate(FileLocation *location)
{
    return NodeCreate(ASTStep, location);
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

static void ASTExpandAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitExpand(visitor, self);
}

static void ASTExpandDestroy(void *_self)
{
    ASTExpand *self = _self;
    free(self->identifier);
}

ASTExpand *NAMidiASTExpandCreate(FileLocation *location)
{
    return NodeCreate(ASTExpand, location);
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
