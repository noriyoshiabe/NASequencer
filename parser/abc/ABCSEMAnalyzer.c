#include "ABCSEMAnalyzer.h"
#include "ABCSEM.h"

#include <stdio.h>
#include <stdlib.h>

#define __Trace__ printf("-- %s:%s - %d\n", __FILE__, __func__, __LINE__);

typedef struct _ABCSEMAnalyzer {
    SEMVisitor visitor;
    Analyzer analyzer;
} ABCSEMAnalyzer;

static Node *process(void *self, Node *node)
{
    node->accept(node, self);
    //return NodeRetain(node);
    return NULL;
}

static void destroy(void *self)
{
    free(self);
}

static void visitFile(void *_self, SEMFile *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitTune(void *_self, SEMTune *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;

    NAIterator *iterator;
   
    iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    iterator = NAMapGetIterator(sem->partMap);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        Node *node = entry->value;
        node->accept(node, self);
    }
}

static void visitKey(void *_self, SEMKey *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;
}

static void visitMeter(void *_self, SEMMeter *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;
}

static void visitUnitNoteLength(void *_self, SEMUnitNoteLength *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;
}

static void visitTempo(void *_self, SEMTempo *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;
}

static void visitPart(void *_self, SEMPart *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;

    NAIterator *iterator;
   
    iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitList(void *_self, SEMList *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitVoice(void *_self, SEMVoice *sem)
{
    __Trace__
}

static void visitDecoration(void *_self, SEMDecoration *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;
}

static void visitNote(void *_self, SEMNote *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;
}

static void visitBrokenRhythm(void *_self, SEMBrokenRhythm *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;
}

static void visitRest(void *_self, SEMRest *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;
}

static void visitRepeat(void *_self, SEMRepeat *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;
}

static void visitBarLine(void *_self, SEMBarLine *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;
}

static void visitTie(void *_self, SEMTie *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;
}

static void visitGraceNote(void *_self, SEMGraceNote *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitTuplet(void *_self, SEMTuplet *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;
}

static void visitChord(void *_self, SEMChord *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitOverlay(void *_self, SEMOverlay *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;
}

static void visitMidiVoice(void *_self, SEMMidiVoice *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;
}

static void visitPropagateAccidental(void *_self, SEMPropagateAccidental *sem)
{
    __Trace__
    ABCSEMAnalyzer *self = _self;
}


Analyzer *ABCSEMAnalyzerCreate(ParseContext *context)
{
    ABCSEMAnalyzer *self = calloc(1, sizeof(ABCSEMAnalyzer));
    
    self->visitor.visitFile = visitFile;
    self->visitor.visitTune = visitTune;
    self->visitor.visitKey = visitKey;
    self->visitor.visitMeter = visitMeter;
    self->visitor.visitUnitNoteLength = visitUnitNoteLength;
    self->visitor.visitTempo = visitTempo;
    self->visitor.visitPart = visitPart;
    self->visitor.visitList = visitList;
    self->visitor.visitVoice = visitVoice;
    self->visitor.visitDecoration = visitDecoration;
    self->visitor.visitNote = visitNote;
    self->visitor.visitBrokenRhythm = visitBrokenRhythm;
    self->visitor.visitRest = visitRest;
    self->visitor.visitRepeat = visitRepeat;
    self->visitor.visitBarLine = visitBarLine;
    self->visitor.visitTie = visitTie;
    self->visitor.visitGraceNote = visitGraceNote;
    self->visitor.visitTuplet = visitTuplet;
    self->visitor.visitChord = visitChord;
    self->visitor.visitOverlay = visitOverlay;
    self->visitor.visitMidiVoice = visitMidiVoice;
    self->visitor.visitPropagateAccidental = visitPropagateAccidental;

    self->analyzer.process = process;
    self->analyzer.destroy = destroy;
    self->analyzer.self = self;

    return &self->analyzer;
}
