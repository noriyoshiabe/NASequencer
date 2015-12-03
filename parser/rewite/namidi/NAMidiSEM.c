#include "NAMidiSEM.h"

#include <stdlib.h>

static void SEMListAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitList(visitor, self);
}

static void SEMListDestroy(void *self)
{
}

SEMList *SEMListCreate(FileLocation *location)
{
    return NodeCreate(SEMList, location);
}

static void SEMResolutionAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitResolution(visitor, self);
}

static void SEMResolutionDestroy(void *self)
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

static void SEMTitleDestroy(void *self)
{
}

SEMTitle *SEMTitleCreate(FileLocation *location)
{
    return NodeCreate(SEMTitle, location);
}

static void SEMTempoAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitTempo(visitor, self);
}

static void SEMTempoDestroy(void *self)
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

static void SEMTimeDestroy(void *self)
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

static void SEMKeyDestroy(void *self)
{
}

SEMKey *SEMKeyCreate(FileLocation *location)
{
    return NodeCreate(SEMKey, location);
}

static void SEMMarkerAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitMarker(visitor, self);
}

static void SEMMarkerDestroy(void *self)
{
}

SEMMarker *SEMMarkerCreate(FileLocation *location)
{
    return NodeCreate(SEMMarker, location);
}

static void SEMChannelAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitChannel(visitor, self);
}

static void SEMChannelDestroy(void *self)
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

static void SEMVoiceDestroy(void *self)
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

static void SEMSynthDestroy(void *self)
{
}

SEMSynth *SEMSynthCreate(FileLocation *location)
{
    return NodeCreate(SEMSynth, location);
}

static void SEMVolumeAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitVolume(visitor, self);
}

static void SEMVolumeDestroy(void *self)
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

static void SEMPanDestroy(void *self)
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

static void SEMChorusDestroy(void *self)
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

static void SEMReverbDestroy(void *self)
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

static void SEMTransposeDestroy(void *self)
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

static void SEMRestDestroy(void *self)
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

static void SEMNoteDestroy(void *self)
{
}

SEMNote *SEMNoteCreate(FileLocation *location)
{
    return NodeCreate(SEMNote, location);
}

static void SEMPatternAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitPattern(visitor, self);
}

static void SEMPatternDestroy(void *self)
{
}

SEMPattern *SEMPatternCreate(FileLocation *location)
{
    return NodeCreate(SEMPattern, location);
}

static void SEMContextAccept(void *self, void *visitor)
{
    ((SEMVisitor *)visitor)->visitContext(visitor, self);
}

static void SEMContextDestroy(void *self)
{
}

SEMContext *SEMContextCreate(FileLocation *location)
{
    return NodeCreate(SEMContext, location);
}
