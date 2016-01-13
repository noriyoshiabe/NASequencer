#include "MMLAST.h"

#include <stdlib.h>

static void ASTRootAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitRoot(visitor, self);
}

static void ASTRootDestroy(void *self)
{
}

ASTRoot *MMLASTRootCreate(FileLocation *location)
{
    return NodeCreate(ASTRoot, location);
}

static void ASTTimebaseAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTimebase(visitor, self);
}

static void ASTTimebaseDestroy(void *self)
{
}

ASTTimebase *MMLASTTimebaseCreate(FileLocation *location)
{
    return NodeCreate(ASTTimebase, location);
}

static void ASTTitleAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTitle(visitor, self);
}

static void ASTTitleDestroy(void *_self)
{
    ASTTitle *self = self;
    free(self->title);
}

ASTTitle *MMLASTTitleCreate(FileLocation *location)
{
    return NodeCreate(ASTTitle, location);
}

static void ASTCopyrightAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitCopyright(visitor, self);
}

static void ASTCopyrightDestroy(void *_self)
{
    ASTCopyright *self = self;
    free(self->text);
}

ASTCopyright *MMLASTCopyrightCreate(FileLocation *location)
{
    return NodeCreate(ASTCopyright, location);
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

ASTMarker *MMLASTMarkerCreate(FileLocation *location)
{
    return NodeCreate(ASTMarker, location);
}

static void ASTVelocityReverseAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitVelocityReverse(visitor, self);
}

static void ASTVelocityReverseDestroy(void *self)
{
}

ASTVelocityReverse *MMLASTVelocityReverseCreate(FileLocation *location)
{
    return NodeCreate(ASTVelocityReverse, location);
}


static void ASTOctaveReverseAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitOctaveReverse(visitor, self);
}

static void ASTOctaveReverseDestroy(void *self)
{
}

ASTOctaveReverse *MMLASTOctaveReverseCreate(FileLocation *location)
{
    return NodeCreate(ASTOctaveReverse, location);
}

static void ASTChannelAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitChannel(visitor, self);
}

static void ASTChannelDestroy(void *self)
{
}

ASTChannel *MMLASTChannelCreate(FileLocation *location)
{
    return NodeCreate(ASTChannel, location);
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

ASTSynth *MMLASTSynthCreate(FileLocation *location)
{
    return NodeCreate(ASTSynth, location);
}

static void ASTBankSelectAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitBankSelect(visitor, self);
}

static void ASTBankSelectDestroy(void *self)
{
}

ASTBankSelect *MMLASTBankSelectCreate(FileLocation *location)
{
    return NodeCreate(ASTBankSelect, location);
}

static void ASTProgramChangeAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitProgramChange(visitor, self);
}

static void ASTProgramChangeDestroy(void *self)
{
}

ASTProgramChange *MMLASTProgramChangeCreate(FileLocation *location)
{
    return NodeCreate(ASTProgramChange, location);
}

static void ASTVolumeAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitVolume(visitor, self);
}

static void ASTVolumeDestroy(void *self)
{
}

ASTVolume *MMLASTVolumeCreate(FileLocation *location)
{
    return NodeCreate(ASTVolume, location);
}

static void ASTChorusAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitChorus(visitor, self);
}

static void ASTChorusDestroy(void *self)
{
}

ASTChorus *MMLASTChorusCreate(FileLocation *location)
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

ASTReverb *MMLASTReverbCreate(FileLocation *location)
{
    return NodeCreate(ASTReverb, location);
}

static void ASTExpressionAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitExpression(visitor, self);
}

static void ASTExpressionDestroy(void *self)
{
}

ASTExpression *MMLASTExpressionCreate(FileLocation *location)
{
    return NodeCreate(ASTExpression, location);
}

static void ASTPanAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitPan(visitor, self);
}

static void ASTPanDestroy(void *self)
{
}

ASTPan *MMLASTPanCreate(FileLocation *location)
{
    return NodeCreate(ASTPan, location);
}

static void ASTDetuneAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitDetune(visitor, self);
}

static void ASTDetuneDestroy(void *self)
{
}

ASTDetune *MMLASTDetuneCreate(FileLocation *location)
{
    return NodeCreate(ASTDetune, location);
}

static void ASTTempoAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTempo(visitor, self);
}

static void ASTTempoDestroy(void *self)
{
}

ASTTempo *MMLASTTempoCreate(FileLocation *location)
{
    return NodeCreate(ASTTempo, location);
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

ASTNote *MMLASTNoteCreate(FileLocation *location)
{
    return NodeCreate(ASTNote, location);
}

static void ASTRestAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitRest(visitor, self);
}

static void ASTRestDestroy(void *_self)
{
    ASTRest *self = _self;
    free(self->restString);
}

ASTRest *MMLASTRestCreate(FileLocation *location)
{
    return NodeCreate(ASTRest, location);
}

static void ASTOctaveAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitOctave(visitor, self);
}

static void ASTOctaveDestroy(void *self)
{
}

ASTOctave *MMLASTOctaveCreate(FileLocation *location)
{
    return NodeCreate(ASTOctave, location);
}

static void ASTTranseposeAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTransepose(visitor, self);
}

static void ASTTranseposeDestroy(void *self)
{
}

ASTTransepose *MMLASTTranseposeCreate(FileLocation *location)
{
    return NodeCreate(ASTTransepose, location);
}

static void ASTTieAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTie(visitor, self);
}

static void ASTTieDestroy(void *self)
{
}

ASTTie *MMLASTTieCreate(FileLocation *location)
{
    return NodeCreate(ASTTie, location);
}

static void ASTLengthAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitLength(visitor, self);
}

static void ASTLengthDestroy(void *self)
{
}

ASTLength *MMLASTLengthCreate(FileLocation *location)
{
    return NodeCreate(ASTLength, location);
}

static void ASTGatetimeAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitGatetime(visitor, self);
}

static void ASTGatetimeDestroy(void *self)
{
}

ASTGatetime *MMLASTGatetimeCreate(FileLocation *location)
{
    return NodeCreate(ASTGatetime, location);
}

static void ASTVelocityAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitVelocity(visitor, self);
}

static void ASTVelocityDestroy(void *self)
{
}

ASTVelocity *MMLASTVelocityCreate(FileLocation *location)
{
    return NodeCreate(ASTVelocity, location);
}

static void ASTTupletAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTuplet(visitor, self);
}

static void ASTTupletDestroy(void *_self)
{
    ASTTuplet *self = _self;
    free(self->lengthString);
}

ASTTuplet *MMLASTTupletCreate(FileLocation *location)
{
    return NodeCreate(ASTTuplet, location);
}

static void ASTTrackChangeAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitTrackChange(visitor, self);
}

static void ASTTrackChangeDestroy(void *self)
{
}

ASTTrackChange *MMLASTTrackChangeCreate(FileLocation *location)
{
    return NodeCreate(ASTTrackChange, location);
}

static void ASTRepeatAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitRepeat(visitor, self);
}

static void ASTRepeatDestroy(void *self)
{
}

ASTRepeat *MMLASTRepeatCreate(FileLocation *location)
{
    return NodeCreate(ASTRepeat, location);
}

static void ASTRepeatBreakAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitRepeatBreak(visitor, self);
}

static void ASTRepeatBreakDestroy(void *self)
{
}

ASTRepeatBreak *MMLASTRepeatBreakCreate(FileLocation *location)
{
    return NodeCreate(ASTRepeatBreak, location);
}

static void ASTChordAccept(void *self, void *visitor)
{
    ((ASTVisitor *)visitor)->visitChord(visitor, self);
}

static void ASTChordDestroy(void *self)
{
}

ASTChord *MMLASTChordCreate(FileLocation *location)
{
    return NodeCreate(ASTChord, location);
}
