#include "MMLSEMAnalyzer.h"
#include "MMLSEM.h"
#include <NALog.h>

#include <stdlib.h>

typedef struct _MMLSEMAnalyzer {
    SEMVisitor visitor;
    Analyzer analyzer;
} MMLSEMAnalyzer;

static Node *process(void *_self, Node *node)
{
    MMLSEMAnalyzer *self = _self;
    node->accept(node, self);
    return NULL;
}

static void destroy(void *self)
{
    free(self);
}

static void visitList(void *_self, SEMList *sem)
{
    MMLSEMAnalyzer *self = _self;

    __Trace__

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitTimebase(void *self, SEMTimebase *sem)
{
    __Trace__
}

static void visitTitle(void *self, SEMTitle *sem)
{
    __Trace__
}

static void visitCopyright(void *self, SEMCopyright *sem)
{
    __Trace__
}

static void visitMarker(void *self, SEMMarker *sem)
{
    __Trace__
}

static void visitVelocityReverse(void *self, SEMVelocityReverse *sem)
{
    __Trace__
}

static void visitOctaveReverse(void *self, SEMOctaveReverse *sem)
{
    __Trace__
}

static void visitChannel(void *self, SEMChannel *sem)
{
    __Trace__
}

static void visitSynth(void *self, SEMSynth *sem)
{
    __Trace__
}

static void visitBankSelect(void *self, SEMBankSelect *sem)
{
    __Trace__
}

static void visitProgramChange(void *self, SEMProgramChange *sem)
{
    __Trace__
}

static void visitVolume(void *self, SEMVolume *sem)
{
    __Trace__
}

static void visitChorus(void *self, SEMChorus *sem)
{
    __Trace__
}

static void visitReverb(void *self, SEMReverb *sem)
{
    __Trace__
}

static void visitExpression(void *self, SEMExpression *sem)
{
    __Trace__
}

static void visitPan(void *self, SEMPan *sem)
{
    __Trace__
}

static void visitDetune(void *self, SEMDetune *sem)
{
    __Trace__
}

static void visitTempo(void *self, SEMTempo *sem)
{
    __Trace__
}

static void visitNote(void *_self, SEMNote *sem)
{
    __Trace__
}

static void visitRest(void *self, SEMRest *sem)
{
    __Trace__
}

static void visitOctave(void *self, SEMOctave *sem)
{
    __Trace__
}

static void visitTransepose(void *self, SEMTranspose *sem)
{
    __Trace__
}

static void visitTie(void *self, SEMTie *sem)
{
    __Trace__
}

static void visitLength(void *self, SEMLength *sem)
{
    __Trace__
}

static void visitGatetime(void *self, SEMGatetime *sem)
{
    __Trace__
}

static void visitVelocity(void *self, SEMVelocity *sem)
{
    __Trace__
}

static void visitTuplet(void *_self, SEMTuplet *sem)
{
    MMLSEMAnalyzer *self = _self;

    __Trace__

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitTrackChange(void *self, SEMTrackChange *sem)
{
    __Trace__
}

static void visitRepeat(void *_self, SEMRepeat *sem)
{
    MMLSEMAnalyzer *self = _self;

    __Trace__

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitRepeatBreak(void *self, SEMRepeatBreak *sem)
{
    __Trace__
}

static void visitChord(void *_self, SEMChord *sem)
{
    MMLSEMAnalyzer *self = _self;

    __Trace__

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

Analyzer *MMLSEMAnalyzerCreate(ParseContext *context)
{
    MMLSEMAnalyzer *self = calloc(1, sizeof(MMLSEMAnalyzer));

    self->visitor.visitList = visitList;
    self->visitor.visitTimebase = visitTimebase;
    self->visitor.visitTitle = visitTitle;
    self->visitor.visitCopyright = visitCopyright;
    self->visitor.visitMarker = visitMarker;
    self->visitor.visitVelocityReverse = visitVelocityReverse;
    self->visitor.visitOctaveReverse = visitOctaveReverse;
    self->visitor.visitChannel = visitChannel;
    self->visitor.visitSynth = visitSynth;
    self->visitor.visitBankSelect = visitBankSelect;
    self->visitor.visitProgramChange = visitProgramChange;
    self->visitor.visitVolume = visitVolume;
    self->visitor.visitChorus = visitChorus;
    self->visitor.visitReverb = visitReverb;
    self->visitor.visitExpression = visitExpression;
    self->visitor.visitPan = visitPan;
    self->visitor.visitDetune = visitDetune;
    self->visitor.visitTempo = visitTempo;
    self->visitor.visitNote = visitNote;
    self->visitor.visitRest = visitRest;
    self->visitor.visitOctave = visitOctave;
    self->visitor.visitTransepose = visitTransepose;
    self->visitor.visitTie = visitTie;
    self->visitor.visitLength = visitLength;
    self->visitor.visitGatetime = visitGatetime;
    self->visitor.visitVelocity = visitVelocity;
    self->visitor.visitTuplet = visitTuplet;
    self->visitor.visitTrackChange = visitTrackChange;
    self->visitor.visitRepeat = visitRepeat;
    self->visitor.visitRepeatBreak = visitRepeatBreak;
    self->visitor.visitChord = visitChord;

    self->analyzer.process = process;
    self->analyzer.destroy = destroy;
    self->analyzer.self = self;

    return &self->analyzer;
}

