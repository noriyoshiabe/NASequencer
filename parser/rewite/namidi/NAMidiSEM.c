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

SEMList *SEMListCreate(FileLocation *location)
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

SEMResolution *SEMResolutionCreate(FileLocation *location)
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

SEMTitle *SEMTitleCreate(FileLocation *location)
{
    return NodeCreate(SEMTitle, location);
}

static void SEMTempoAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTempo(visitor, self);
}

static void SEMTempoDestroy(void *_self)
{
}

SEMTempo *SEMTempoCreate(FileLocation *location)
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

SEMTime *SEMTimeCreate(FileLocation *location)
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

SEMKey *SEMKeyCreate(FileLocation *location)
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

SEMMarker *SEMMarkerCreate(FileLocation *location)
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

SEMChannel *SEMChannelCreate(FileLocation *location)
{
    return NodeCreate(SEMChannel, location);
}

static void SEMVoiceAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitVoice(visitor, self);
}

static void SEMVoiceDestroy(void *_self)
{
}

SEMVoice *SEMVoiceCreate(FileLocation *location)
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

SEMSynth *SEMSynthCreate(FileLocation *location)
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

SEMVolume *SEMVolumeCreate(FileLocation *location)
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

SEMPan *SEMPanCreate(FileLocation *location)
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

SEMChorus *SEMChorusCreate(FileLocation *location)
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

SEMReverb *SEMReverbCreate(FileLocation *location)
{
    return NodeCreate(SEMReverb, location);
}

static void SEMTransposeAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTranspose(visitor, self);
}

static void SEMTransposeDestroy(void *_self)
{
}

SEMTranspose *SEMTransposeCreate(FileLocation *location)
{
    return NodeCreate(SEMTranspose, location);
}

static void SEMRestAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitRest(visitor, self);
}

static void SEMRestDestroy(void *_self)
{
}

SEMRest *SEMRestCreate(FileLocation *location)
{
    return NodeCreate(SEMRest, location);
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

SEMNote *SEMNoteCreate(FileLocation *location)
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

SEMPattern *SEMPatternCreate(FileLocation *location)
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

SEMContext *SEMContextCreate(FileLocation *location)
{
    SEMContext *self = NodeCreate(SEMContext, location);
    self->node.children = NAArrayCreate(4, NULL);
    self->ctxIdList = NAArrayCreate(4, NULL);
    return self;
}
