#include "NAMidiSEM.h"

#include <stdlib.h>

static void SEMListAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitList(visitor, self);
}

static void SEMListDestroy(void *_self)
{
    SEMList *self = _self;
    if (self->identifier) {
        free(self->identifier);
    }
    
    NAMapTraverseValue(self->patternMap, NodeRelease);
    NAMapDestroy(self->patternMap);
}

SEMList *NAMidiSEMListCreate(FileLocation *location)
{
    SEMList *self = NodeCreate(SEMList, location);
    self->node.children = NAArrayCreate(4, NULL);
    self->patternMap = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    return self;
}

static void SEMResolutionAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitResolution(visitor, self);
}

static void SEMResolutionDestroy(void *_self)
{
}

SEMResolution *NAMidiSEMResolutionCreate(FileLocation *location)
{
    return NodeCreate(SEMResolution, location);
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

SEMTitle *NAMidiSEMTitleCreate(FileLocation *location)
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

SEMCopyright *NAMidiSEMCopyrightCreate(FileLocation *location)
{
    return NodeCreate(SEMCopyright, location);
}

static void SEMTempoAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTempo(visitor, self);
}

static void SEMTempoDestroy(void *_self)
{
}

SEMTempo *NAMidiSEMTempoCreate(FileLocation *location)
{
    return NodeCreate(SEMTempo, location);
}

static void SEMTimeAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTime(visitor, self);
}

static void SEMTimeDestroy(void *_self)
{
}

SEMTime *NAMidiSEMTimeCreate(FileLocation *location)
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
    NoteTableRelease(self->noteTable);
}

SEMKey *NAMidiSEMKeyCreate(FileLocation *location)
{
    return NodeCreate(SEMKey, location);
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

SEMMarker *NAMidiSEMMarkerCreate(FileLocation *location)
{
    return NodeCreate(SEMMarker, location);
}

static void SEMChannelAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitChannel(visitor, self);
}

static void SEMChannelDestroy(void *_self)
{
}

SEMChannel *NAMidiSEMChannelCreate(FileLocation *location)
{
    return NodeCreate(SEMChannel, location);
}

static void SEMVelocityAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitVelocity(visitor, self);
}

static void SEMVelocityDestroy(void *_self)
{
}

SEMVelocity *NAMidiSEMVelocityCreate(FileLocation *location)
{
    return NodeCreate(SEMVelocity, location);
}

static void SEMGatetimeAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitGatetime(visitor, self);
}

static void SEMGatetimeDestroy(void *_self)
{
}

SEMGatetime *NAMidiSEMGatetimeCreate(FileLocation *location)
{
    return NodeCreate(SEMGatetime, location);
}

static void SEMVoiceAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitVoice(visitor, self);
}

static void SEMVoiceDestroy(void *_self)
{
}

SEMVoice *NAMidiSEMVoiceCreate(FileLocation *location)
{
    return NodeCreate(SEMVoice, location);
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

SEMSynth *NAMidiSEMSynthCreate(FileLocation *location)
{
    return NodeCreate(SEMSynth, location);
}

static void SEMVolumeAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitVolume(visitor, self);
}

static void SEMVolumeDestroy(void *_self)
{
}

SEMVolume *NAMidiSEMVolumeCreate(FileLocation *location)
{
    return NodeCreate(SEMVolume, location);
}

static void SEMPanAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitPan(visitor, self);
}

static void SEMPanDestroy(void *_self)
{
}

SEMPan *NAMidiSEMPanCreate(FileLocation *location)
{
    return NodeCreate(SEMPan, location);
}

static void SEMChorusAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitChorus(visitor, self);
}

static void SEMChorusDestroy(void *_self)
{
}

SEMChorus *NAMidiSEMChorusCreate(FileLocation *location)
{
    return NodeCreate(SEMChorus, location);
}

static void SEMReverbAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitReverb(visitor, self);
}

static void SEMReverbDestroy(void *_self)
{
}

SEMReverb *NAMidiSEMReverbCreate(FileLocation *location)
{
    return NodeCreate(SEMReverb, location);
}

static void SEMExpressionAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitExpression(visitor, self);
}

static void SEMExpressionDestroy(void *_self)
{
}

SEMExpression *NAMidiSEMExpressionCreate(FileLocation *location)
{
    return NodeCreate(SEMExpression, location);
}

static void SEMTransposeAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTranspose(visitor, self);
}

static void SEMTransposeDestroy(void *_self)
{
}

SEMTranspose *NAMidiSEMTransposeCreate(FileLocation *location)
{
    return NodeCreate(SEMTranspose, location);
}

static void SEMStepAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitStep(visitor, self);
}

static void SEMStepDestroy(void *_self)
{
}

SEMStep *NAMidiSEMStepCreate(FileLocation *location)
{
    return NodeCreate(SEMStep, location);
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

SEMNote *NAMidiSEMNoteCreate(FileLocation *location)
{
    return NodeCreate(SEMNote, location);
}

static void SEMPatternAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitPattern(visitor, self);
}

static void SEMPatternDestroy(void *_self)
{
    SEMPattern *self = _self;
    free(self->identifier);
    NAArrayTraverse(self->ctxIdList, free);
    NAArrayDestroy(self->ctxIdList);
}

SEMPattern *NAMidiSEMPatternCreate(FileLocation *location)
{
    SEMPattern *self = NodeCreate(SEMPattern, location);
    self->node.children = NAArrayCreate(4, NULL);
    self->ctxIdList = NAArrayCreate(4, NULL);
    return self;
}

static void SEMContextAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitContext(visitor, self);
}

static void SEMContextDestroy(void *_self)
{
    SEMContext *self = _self;
    NAArrayTraverse(self->ctxIdList, free);
    NAArrayDestroy(self->ctxIdList);
    NodeRelease(self->list);
}

SEMContext *NAMidiSEMContextCreate(FileLocation *location)
{
    SEMContext *self = NodeCreate(SEMContext, location);
    self->node.children = NAArrayCreate(4, NULL);
    self->ctxIdList = NAArrayCreate(4, NULL);
    return self;
}
