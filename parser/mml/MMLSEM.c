#include "MMLSEM.h"

#include <stdlib.h>

static void SEMListAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitList(visitor, self);
}

static void SEMListDestroy(void *_self)
{
    SEMList *self = _self;
}

SEMList *MMLSEMListCreate(FileLocation *location)
{
    SEMList *self = NodeCreate(SEMList, location);
    self->node.children = NAArrayCreate(4, NULL);
    return self;
}

static void SEMTimebaseAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTimebase(visitor, self);
}

static void SEMTimebaseDestroy(void *self)
{
}

SEMTimebase *MMLSEMTimebaseCreate(FileLocation *location)
{
    return NodeCreate(SEMTimebase, location);
}

static void SEMTitleAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTitle(visitor, self);
}

static void SEMTitleDestroy(void *_self)
{
    SEMTitle *self = _self;
    free(self->title);
}

SEMTitle *MMLSEMTitleCreate(FileLocation *location)
{
    return NodeCreate(SEMTitle, location);
}

static void SEMCopyrightAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitCopyright(visitor, self);
}

static void SEMCopyrightDestroy(void *_self)
{
    SEMCopyright *self = _self;
    free(self->text);
}

SEMCopyright *MMLSEMCopyrightCreate(FileLocation *location)
{
    return NodeCreate(SEMCopyright, location);
}

static void SEMMarkerAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitMarker(visitor, self);
}

static void SEMMarkerDestroy(void *_self)
{
    SEMMarker *self = _self;
    free(self->text);
}

SEMMarker *MMLSEMMarkerCreate(FileLocation *location)
{
    return NodeCreate(SEMMarker, location);
}

static void SEMVelocityReverseAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitVelocityReverse(visitor, self);
}

static void SEMVelocityReverseDestroy(void *self)
{
}

SEMVelocityReverse *MMLSEMVelocityReverseCreate(FileLocation *location)
{
    return NodeCreate(SEMVelocityReverse, location);
}


static void SEMOctaveReverseAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitOctaveReverse(visitor, self);
}

static void SEMOctaveReverseDestroy(void *self)
{
}

SEMOctaveReverse *MMLSEMOctaveReverseCreate(FileLocation *location)
{
    return NodeCreate(SEMOctaveReverse, location);
}

static void SEMChannelAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitChannel(visitor, self);
}

static void SEMChannelDestroy(void *self)
{
}

SEMChannel *MMLSEMChannelCreate(FileLocation *location)
{
    return NodeCreate(SEMChannel, location);
}

static void SEMSynthAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitSynth(visitor, self);
}

static void SEMSynthDestroy(void *_self)
{
    SEMSynth *self = _self;
    free(self->name);
}

SEMSynth *MMLSEMSynthCreate(FileLocation *location)
{
    return NodeCreate(SEMSynth, location);
}

static void SEMBankSelectAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitBankSelect(visitor, self);
}

static void SEMBankSelectDestroy(void *self)
{
}

SEMBankSelect *MMLSEMBankSelectCreate(FileLocation *location)
{
    return NodeCreate(SEMBankSelect, location);
}

static void SEMProgramChangeAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitProgramChange(visitor, self);
}

static void SEMProgramChangeDestroy(void *self)
{
}

SEMProgramChange *MMLSEMProgramChangeCreate(FileLocation *location)
{
    return NodeCreate(SEMProgramChange, location);
}

static void SEMVolumeAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitVolume(visitor, self);
}

static void SEMVolumeDestroy(void *self)
{
}

SEMVolume *MMLSEMVolumeCreate(FileLocation *location)
{
    return NodeCreate(SEMVolume, location);
}

static void SEMChorusAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitChorus(visitor, self);
}

static void SEMChorusDestroy(void *self)
{
}

SEMChorus *MMLSEMChorusCreate(FileLocation *location)
{
    return NodeCreate(SEMChorus, location);
}

static void SEMReverbAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitReverb(visitor, self);
}

static void SEMReverbDestroy(void *self)
{
}

SEMReverb *MMLSEMReverbCreate(FileLocation *location)
{
    return NodeCreate(SEMReverb, location);
}

static void SEMExpressionAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitExpression(visitor, self);
}

static void SEMExpressionDestroy(void *self)
{
}

SEMExpression *MMLSEMExpressionCreate(FileLocation *location)
{
    return NodeCreate(SEMExpression, location);
}

static void SEMPanAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitPan(visitor, self);
}

static void SEMPanDestroy(void *self)
{
}

SEMPan *MMLSEMPanCreate(FileLocation *location)
{
    return NodeCreate(SEMPan, location);
}

static void SEMDetuneAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitDetune(visitor, self);
}

static void SEMDetuneDestroy(void *self)
{
}

SEMDetune *MMLSEMDetuneCreate(FileLocation *location)
{
    return NodeCreate(SEMDetune, location);
}

static void SEMTempoAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTempo(visitor, self);
}

static void SEMTempoDestroy(void *self)
{
}

SEMTempo *MMLSEMTempoCreate(FileLocation *location)
{
    return NodeCreate(SEMTempo, location);
}

static void SEMNoteAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitNote(visitor, self);
}

static void SEMNoteDestroy(void *_self)
{
    SEMNote *self = _self;
    free(self->noteString);
}

SEMNote *MMLSEMNoteCreate(FileLocation *location)
{
    return NodeCreate(SEMNote, location);
}

static void SEMRestAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitRest(visitor, self);
}

static void SEMRestDestroy(void *self)
{
}

SEMRest *MMLSEMRestCreate(FileLocation *location)
{
    return NodeCreate(SEMRest, location);
}

static void SEMOctaveAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitOctave(visitor, self);
}

static void SEMOctaveDestroy(void *self)
{
}

SEMOctave *MMLSEMOctaveCreate(FileLocation *location)
{
    return NodeCreate(SEMOctave, location);
}

static void SEMTranseposeAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTransepose(visitor, self);
}

static void SEMTranseposeDestroy(void *self)
{
}

SEMTransepose *MMLSEMTranseposeCreate(FileLocation *location)
{
    return NodeCreate(SEMTransepose, location);
}

static void SEMTieAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTie(visitor, self);
}

static void SEMTieDestroy(void *self)
{
}

SEMTie *MMLSEMTieCreate(FileLocation *location)
{
    return NodeCreate(SEMTie, location);
}

static void SEMLengthAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitLength(visitor, self);
}

static void SEMLengthDestroy(void *self)
{
}

SEMLength *MMLSEMLengthCreate(FileLocation *location)
{
    return NodeCreate(SEMLength, location);
}

static void SEMGatetimeAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitGatetime(visitor, self);
}

static void SEMGatetimeDestroy(void *self)
{
}

SEMGatetime *MMLSEMGatetimeCreate(FileLocation *location)
{
    return NodeCreate(SEMGatetime, location);
}

static void SEMVelocityAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitVelocity(visitor, self);
}

static void SEMVelocityDestroy(void *self)
{
}

SEMVelocity *MMLSEMVelocityCreate(FileLocation *location)
{
    return NodeCreate(SEMVelocity, location);
}

static void SEMTupletAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTuplet(visitor, self);
}

static void SEMTupletDestroy(void *self)
{
}

SEMTuplet *MMLSEMTupletCreate(FileLocation *location)
{
    return NodeCreate(SEMTuplet, location);
}

static void SEMTrackChangeAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTrackChange(visitor, self);
}

static void SEMTrackChangeDestroy(void *self)
{
}

SEMTrackChange *MMLSEMTrackChangeCreate(FileLocation *location)
{
    return NodeCreate(SEMTrackChange, location);
}

static void SEMRepeatAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitRepeat(visitor, self);
}

static void SEMRepeatDestroy(void *self)
{
}

SEMRepeat *MMLSEMRepeatCreate(FileLocation *location)
{
    return NodeCreate(SEMRepeat, location);
}

static void SEMRepeatBreakAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitRepeatBreak(visitor, self);
}

static void SEMRepeatBreakDestroy(void *self)
{
}

SEMRepeatBreak *MMLSEMRepeatBreakCreate(FileLocation *location)
{
    return NodeCreate(SEMRepeatBreak, location);
}

static void SEMChordAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitChord(visitor, self);
}

static void SEMChordDestroy(void *self)
{
}

SEMChord *MMLSEMChordCreate(FileLocation *location)
{
    return NodeCreate(SEMChord, location);
}
