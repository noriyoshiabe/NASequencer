#include "ABCSEM.h"

#include <stdlib.h>

#define IF(p, destroy) do { if (p) { destroy(p); } } while (0)

static void SEMFileAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitFile(visitor, self);
}

static void SEMFileDestroy(void *self)
{
}

SEMFile *ABCSEMFileCreate(FileLocation *location)
{
    return NodeCreate(SEMFile, location);
}

static void SEMTuneAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTune(visitor, self);
}

static void SEMTuneDestroy(void *_self)
{
    SEMTune *self = _self;
    IF(self->partSequence, free);

    NAArrayTraverse(self->titleList, free);
    NAArrayDestroy(self->titleList);

    NAMapTraverseValue(self->partMap, NodeRelease);
    NAMapDestroy(self->partMap);
}

SEMTune *ABCSEMTuneCreate(FileLocation *location)
{
    SEMTune *self = NodeCreate(SEMTune, location);
    self->titleList = NAArrayCreate(4, NADescriptionCString);
    self->partMap = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    return self;
}

static void SEMTempoAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTempo(visitor, self);
}

static void SEMTempoDestroy(void *self)
{
}

SEMTempo *ABCSEMTempoCreate(FileLocation *location)
{
    return NodeCreate(SEMTempo, location);
}

static void SEMTimeAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTime(visitor, self);
}

static void SEMTimeDestroy(void *self)
{
}

SEMTime *ABCSEMTimeCreate(FileLocation *location)
{
    return NodeCreate(SEMTime, location);
}

static void SEMKeyAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitKey(visitor, self);
}

static void SEMKeyDestroy(void *_self)
{
    SEMKey *self = _self;
    IF(self->noteTable, NoteTableRelease);
}

SEMKey *ABCSEMKeyCreate(FileLocation *location)
{
    return NodeCreate(SEMKey, location);
}

static void SEMUnitNoteLengthAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitUnitNoteLength(visitor, self);
}

static void SEMUnitNoteLengthDestroy(void *self)
{
}

SEMUnitNoteLength *ABCSEMUnitNoteLengthCreate(FileLocation *location)
{
    return NodeCreate(SEMUnitNoteLength, location);
}

static void SEMPartAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitPart(visitor, self);
}

static void SEMPartDestroy(void *_self)
{
    SEMPart *self = _self;
    IF(self->identifier, free);
}

SEMPart *ABCSEMPartCreate(FileLocation *location)
{
    return NodeCreate(SEMPart, location);
}

static void SEMVoiceAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitVoice(visitor, self);
}

static void SEMVoiceDestroy(void *_self)
{
    SEMVoice *self = _self;
    IF(self->identifier, free);
}

SEMVoice *ABCSEMVoiceCreate(FileLocation *location)
{
    return NodeCreate(SEMVoice, location);
}

static void SEMNoteAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitNote(visitor, self);
}

static void SEMNoteDestroy(void *self)
{
}

SEMNote *ABCSEMNoteCreate(FileLocation *location)
{
    return NodeCreate(SEMNote, location);
}

static void SEMBrokenRhythmAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitBrokenRhythm(visitor, self);
}

static void SEMBrokenRhythmDestroy(void *self)
{
}

SEMBrokenRhythm *ABCSEMBrokenRhythmCreate(FileLocation *location)
{
    return NodeCreate(SEMBrokenRhythm, location);
}

static void SEMRestAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitRest(visitor, self);
}

static void SEMRestDestroy(void *self)
{
}

SEMRest *ABCSEMRestCreate(FileLocation *location)
{
    return NodeCreate(SEMRest, location);
}

static void SEMRepeatAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitRepeat(visitor, self);
}

static void SEMRepeatDestroy(void *self)
{
}

SEMRepeat *ABCSEMRepeatCreate(FileLocation *location)
{
    return NodeCreate(SEMRepeat, location);
}

static void SEMBarLineAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitBarLine(visitor, self);
}

static void SEMBarLineDestroy(void *self)
{
}

SEMBarLine *ABCSEMBarLineCreate(FileLocation *location)
{
    return NodeCreate(SEMBarLine, location);
}

static void SEMTieAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTie(visitor, self);
}

static void SEMTieDestroy(void *self)
{
}

SEMTie *ABCSEMTieCreate(FileLocation *location)
{
    return NodeCreate(SEMTie, location);
}

static void SEMGraceNoteAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitGraceNote(visitor, self);
}

static void SEMGraceNoteDestroy(void *self)
{
}

SEMGraceNote *ABCSEMGraceNoteCreate(FileLocation *location)
{
    return NodeCreate(SEMGraceNote, location);
}

static void SEMTupletAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTuplet(visitor, self);
}

static void SEMTupletDestroy(void *self)
{
}

SEMTuplet *ABCSEMTupletCreate(FileLocation *location)
{
    return NodeCreate(SEMTuplet, location);
}

static void SEMChordAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitChord(visitor, self);
}

static void SEMChordDestroy(void *self)
{
}

SEMChord *ABCSEMChordCreate(FileLocation *location)
{
    return NodeCreate(SEMChord, location);
}

static void SEMOverlayAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitOverlay(visitor, self);
}

static void SEMOverlayDestroy(void *self)
{
}

SEMOverlay *ABCSEMOverlayCreate(FileLocation *location)
{
    return NodeCreate(SEMOverlay, location);
}

static void SEMMidiVoiceAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitMidiVoice(visitor, self);
}

static void SEMMidiVoiceDestroy(void *self)
{
}

SEMMidiVoice *ABCSEMMidiVoiceCreate(FileLocation *location)
{
    return NodeCreate(SEMMidiVoice, location);
}

static void SEMPropagateAccidentalAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitPropagateAccidental(visitor, self);
}

static void SEMPropagateAccidentalDestroy(void *self)
{
}

SEMPropagateAccidental *ABCSEMPropagateAccidentalCreate(FileLocation *location)
{
    return NodeCreate(SEMPropagateAccidental, location);
}