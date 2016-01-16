#include "MMLSEMAnalyzer.h"
#include "MMLSEM.h"
#include "MMLParser.h"
#include "NAArray.h"
#include "NAStack.h"
#include "NACString.h"
#include "NALog.h"

#include <stdlib.h>

#define appendError(self, sem, ...) self->context->appendError(self->context, &sem->node.location, __VA_ARGS__)
#define isValidRange(v, from, to) (from <= v && v <= to)

typedef struct _Note {
    struct _Note *next;
} Note;

typedef struct _RepeatContext {
    int current;
    SEMRepeat *repeat;
} RepeatContext;

typedef struct _MMLSEMAnalyzer {
    SEMVisitor visitor;
    Analyzer analyzer;
    ParseContext *context;
    SequenceBuilder *builder;

    int timebase;
    bool velocityReverse;
    bool octaveReverse;
    int tick;
    int channel;
    int msb;
    int lsb;
    int octave;
    int transpose;
    bool tie;
    int length;
    struct {
        int rate;
        int minus;
    } gatetime;
    int velocity;

    Note *pendingNote;

    SEMTuplet *tuplet;

    RepeatContext *repeatContext;
    NAStack *repeatContextStack;

    struct {
        Node *timebase;
        Node *title;
        Node *copyright;
    } definedNode;
} MMLSEMAnalyzer;

static Node *process(void *_self, Node *node)
{
    MMLSEMAnalyzer *self = _self;
    node->accept(node, self);
    return NULL;
}

static void destroy(void *_self)
{
    MMLSEMAnalyzer *self = _self;
    NAStackDestroy(self->repeatContextStack);
    free(self);
}

static void visitList(void *_self, SEMList *sem)
{
    MMLSEMAnalyzer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitTimebase(void *_self, SEMTimebase *sem)
{
    MMLSEMAnalyzer *self = _self;
    if (self->definedNode.timebase) {
        FileLocation *loc = &self->definedNode.timebase->location;
        appendError(self, sem, MMLParseErrorAlreadyDefinedWithTimebase, loc->filepath, NACStringFromInteger(loc->line), NACStringFromInteger(loc->column), NULL);
        return;
    }

    self->timebase = sem->timebase;
    self->builder->setResolution(self->builder, sem->timebase);
    self->definedNode.timebase = (Node *)sem;
}

static void visitTitle(void *_self, SEMTitle *sem)
{
    MMLSEMAnalyzer *self = _self;

    if (self->definedNode.title) {
        FileLocation *loc = &self->definedNode.title->location;
        appendError(self, sem, MMLParseErrorAlreadyDefinedWithTitle, loc->filepath, NACStringFromInteger(loc->line), NACStringFromInteger(loc->column), NULL);
        return;
    }

    self->builder->setTitle(self->builder, sem->title);
    self->definedNode.title = (Node *)sem;
}

static void visitCopyright(void *_self, SEMCopyright *sem)
{
    MMLSEMAnalyzer *self = _self;

    if (self->definedNode.copyright) {
        FileLocation *loc = &self->definedNode.copyright->location;
        appendError(self, sem, MMLParseErrorAlreadyDefinedWithCopyright, loc->filepath, NACStringFromInteger(loc->line), NACStringFromInteger(loc->column), NULL);
        return;
    }

    // TODO
    //self->builder->setCopyright(self->builder, sem->text);
    self->definedNode.copyright = (Node *)sem;
}

static void visitMarker(void *_self, SEMMarker *sem)
{
    MMLSEMAnalyzer *self = _self;
    self->builder->appendMarker(self->builder, self->tick, sem->text);
}

static void visitVelocityReverse(void *_self, SEMVelocityReverse *sem)
{
    MMLSEMAnalyzer *self = _self;
    self->velocityReverse = !self->velocityReverse;
}

static void visitOctaveReverse(void *_self, SEMOctaveReverse *sem)
{
    MMLSEMAnalyzer *self = _self;
    self->octaveReverse = !self->octaveReverse;
}

static void visitChannel(void *_self, SEMChannel *sem)
{
    MMLSEMAnalyzer *self = _self;
    self->channel = sem->number - 1;
}

static void visitSynth(void *_self, SEMSynth *sem)
{
    MMLSEMAnalyzer *self = _self;
    self->builder->appendSynth(self->builder, self->tick, self->channel, sem->name);
}

static void visitBankSelect(void *_self, SEMBankSelect *sem)
{
    MMLSEMAnalyzer *self = _self;
    self->msb = sem->msb;
    self->lsb = sem->lsb;
}

static void visitProgramChange(void *_self, SEMProgramChange *sem)
{
    MMLSEMAnalyzer *self = _self;
    self->builder->appendVoice(self->builder, self->tick, self->channel, self->msb, self->lsb, sem->programNo);
}

static void visitVolume(void *_self, SEMVolume *sem)
{
    MMLSEMAnalyzer *self = _self;
    self->builder->appendVolume(self->builder, self->tick, self->channel, sem->value);
}

static void visitChorus(void *_self, SEMChorus *sem)
{
    MMLSEMAnalyzer *self = _self;
    self->builder->appendChorus(self->builder, self->tick, self->channel, sem->value);
}

static void visitReverb(void *_self, SEMReverb *sem)
{
    MMLSEMAnalyzer *self = _self;
    self->builder->appendReverb(self->builder, self->tick, self->channel, sem->value);
}

static void visitExpression(void *_self, SEMExpression *sem)
{
    MMLSEMAnalyzer *self = _self;
    // TODO
    //self->builder->appendExpression(self->builder, self->tick, self->channel, sem->value);
}

static void visitPan(void *_self, SEMPan *sem)
{
    MMLSEMAnalyzer *self = _self;
    self->builder->appendPan(self->builder, self->tick, self->channel, sem->value - 64);
}

static void visitDetune(void *_self, SEMDetune *sem)
{
    MMLSEMAnalyzer *self = _self;
    // TODO
    //self->builder->appendDetune(self->builder, self->tick, self->channel, sem->value);
}

static void visitTempo(void *_self, SEMTempo *sem)
{
    MMLSEMAnalyzer *self = _self;
    self->builder->appendTempo(self->builder, self->tick, sem->tempo);
}

static void visitNote(void *_self, SEMNote *sem)
{
    // TODO
    __Trace__
}

static void visitRest(void *_self, SEMRest *sem)
{
    // TODO
    __Trace__
}

static void visitOctave(void *_self, SEMOctave *sem)
{
    MMLSEMAnalyzer *self = _self;

    if (sem->direction) {
        int shift = '<' == sem->direction ? 1 : -1;
        shift *= self->octaveReverse ? -1 : 1;
        int octave = self->octave + shift;
        if (!isValidRange(octave, -2, 8)) {
            appendError(self, sem, MMLParseErrorInvalidOctave, NACStringFromInteger(octave), NULL);
        }
        else {
            self->octave = octave;
        }
    }
    else {
        self->octave = sem->value;
    }
}

static void visitTransepose(void *_self, SEMTranspose *sem)
{
    MMLSEMAnalyzer *self = _self;

    if (sem->relative) {
        int transpose = self->transpose + sem->value;
        if (!isValidRange(transpose, -64, 64)) {
            appendError(self, sem, MMLParseErrorInvalidTranspose, NACStringFromInteger(transpose), NULL);
        }
        else {
            self->transpose = transpose;
        }
    }
    else {
        self->transpose = sem->value;
    }
}

static void visitTie(void *_self, SEMTie *sem)
{
    // TODO
    __Trace__
}

static void visitLength(void *_self, SEMLength *sem)
{
    MMLSEMAnalyzer *self = _self;
    self->length = sem->length;
}

static void visitGatetime(void *_self, SEMGatetime *sem)
{
    MMLSEMAnalyzer *self = _self;

    if (sem->absolute) {
        self->gatetime.minus = sem->value;
    }
    else {
        self->gatetime.rate = sem->value;
    }
}

static void visitVelocity(void *_self, SEMVelocity *sem)
{
    MMLSEMAnalyzer *self = _self;

    // TODO
}

static void visitTuplet(void *_self, SEMTuplet *sem)
{
    MMLSEMAnalyzer *self = _self;

    // TODO
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

    self->context = context;
    self->builder = context->builder;

    self->repeatContextStack = NAStackCreate(4);

    self->timebase = 480;
    self->channel = 1;
    self->length = 4;
    self->gatetime.rate = 15;
    self->gatetime.minus = 0;
    self->velocity = 100;

    return &self->analyzer;
}

