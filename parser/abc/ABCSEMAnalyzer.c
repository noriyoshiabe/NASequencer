#include "ABCSEMAnalyzer.h"
#include "ABCSEM.h"
#include "NACInteger.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define __Trace__ printf("-- %s:%s - %d\n", __FILE__, __func__, __LINE__);

typedef enum {
    RepeatStateInitial,
    RepeatStateStart,
    RepeatStateEnd,
    RepeatStateNthSearch,
} RepeatState;

typedef struct _RepeatContext {
    int startIndex;
    int nth;
    NASet *passedEndSet;

    RepeatState state;
    int currentIndex;
} RepeatContext;

static RepeatContext *RepeatContextCreate();
static void RepeatContextDestroy(RepeatContext *self);

typedef struct _ABCSEMAnalyzer {
    SEMVisitor visitor;
    Analyzer analyzer;

    ParseContext *context;
    NAMap *repeatMap;

    RepeatContext *repeat;
} ABCSEMAnalyzer;

static Node *process(void *self, Node *node)
{
    node->accept(node, self);
    //return NodeRetain(node);
    return NULL;
}

static void destroy(void *_self)
{
    ABCSEMAnalyzer *self = _self;
    NAMapTraverseValue(self->repeatMap, RepeatContextDestroy);
    NAMapDestroy(self->repeatMap);
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
        SEMPart *part = entry->value;

        NAIterator *_iterator = NAMapGetIterator(part->listMap);
        while (_iterator->hasNext(_iterator)) {
            NAMapEntry *_entry = _iterator->next(_iterator);
            SEMList *list = _entry->value;
            NAMapPut(self->repeatMap, list, RepeatContextCreate());
        }
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

    self->repeat = NAMapGet(self->repeatMap, sem);
    self->repeat->startIndex = 0;
    NASetTraverse(self->repeat->passedEndSet, free);
    NASetRemoveAll(self->repeat->passedEndSet);

REPEAT:
    ++self->repeat->nth;
    self->repeat->state = RepeatStateInitial;
    self->repeat->currentIndex = self->repeat->startIndex;

    NAIterator *iterator = NAArrayGetIteratorWithIndex(sem->node.children, self->repeat->startIndex);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    
        switch (self->repeat->state) {
        case RepeatStateInitial:
            break;
        case RepeatStateStart:
            self->repeat->state = RepeatStateInitial;
            self->repeat->startIndex = self->repeat->currentIndex + 1;
            break;
        case RepeatStateEnd:
            NASetAdd(self->repeat->passedEndSet, NACIntegerFromInteger(self->repeat->currentIndex)); 
            self->repeat->state = RepeatStateInitial;
            goto REPEAT;
        case RepeatStateNthSearch:
            break;
        }
        
        ++self->repeat->currentIndex;
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

    if (RepeatStateInitial != self->repeat->state) {
        return;
    }

    printf("[REPEAT TEST] baseNote=%s\n", BaseNote2String(sem->baseNote));
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

    switch (sem->type) {
    case RepeatStart:
        self->repeat->state = RepeatStateStart;
        self->repeat->nth = 1;
        break;
    case RepeatEnd:
        if (!NASetContains(self->repeat->passedEndSet, &self->repeat->currentIndex)) {
            self->repeat->state = RepeatStateEnd;
        }
        break;
    case RepeatNth:
        if (!NASetContains(sem->nthSet, &self->repeat->nth)) {
            self->repeat->state = RepeatStateNthSearch;
        }
        break;
    }
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

    self->context = context;
    self->repeatMap = NAMapCreate(NAHashAddress, NADescriptionAddress, NADescriptionAddress);

    return &self->analyzer;
}

static RepeatContext *RepeatContextCreate()
{
    RepeatContext *self = calloc(1, sizeof(RepeatContext));
    self->passedEndSet = NASetCreate(NAHashCInteger, NADescriptionCInteger);
    return self;
}

static void RepeatContextDestroy(RepeatContext *self)
{
    NASetTraverse(self->passedEndSet, free);
    NASetDestroy(self->passedEndSet);
}
