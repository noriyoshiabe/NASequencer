#include "MMLSEMAnalyzer.h"
#include "MMLSEM.h"
#include "MMLParser.h"
#include "NoteTable.h"
#include "NAArray.h"
#include "NAStack.h"
#include "NACString.h"
#include "NALog.h"

#include <stdlib.h>
#include <sys/param.h>

#define appendError(self, sem, ...) self->context->appendError(self->context, &sem->node.location, __VA_ARGS__)
#define isValidRange(v, from, to) (from <= v && v <= to)

typedef struct _Note {
    int tick;
    int channel;
    int noteNo;
    int gatetime;
    int velocity;

    bool tied;
} Note;

typedef struct _RepeatContext {
    SEMRepeat *sem;
    int current;
    bool breaked;
} RepeatContext;

static RepeatContext *RepeatContextCreate(SEMRepeat *sem);
static void RepeatContextDestroy(RepeatContext *self);

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
        bool absolute;
    } gatetime;
    struct {
        int value;
        bool absolute;
    } velocity;

    NAArray *pendingNotes;

    SEMTuplet *tuplet;

    RepeatContext *repeatContext;
    NAStack *repeatContextStack;

    bool inChord;
    struct {
        int offset;
        int step;
    } chord;

    struct {
        Node *timebase;
        Node *title;
        Node *copyright;
    } definedNode;

    NoteTable *noteTable;

    int sequenceLength;
} MMLSEMAnalyzer;

static void appendPendingNote(MMLSEMAnalyzer *self, int tick, int channel, int noteNo, int gatetime, int velocity);
static void flushPendingNote(MMLSEMAnalyzer *self);
static bool processTie(MMLSEMAnalyzer *self, int step, int channel, int noteNo);
static void preprocessTie(MMLSEMAnalyzer *self);
static void flushPreviousPendingNoteWithoutTie(MMLSEMAnalyzer *self, int tick);

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
    NAArrayDestroy(self->pendingNotes);
    NoteTableRelease(self->noteTable);
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

    flushPendingNote(self);
    self->sequenceLength = MAX(self->sequenceLength, self->tick);
    self->builder->setLength(self->builder, self->sequenceLength);
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

    self->builder->appendTitle(self->builder, 0, sem->title);
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

    self->builder->appendCopyright(self->builder, 0, sem->text);
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
    self->channel = sem->number;
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
    self->builder->appendExpression(self->builder, self->tick, self->channel, sem->value);
}

static void visitPan(void *_self, SEMPan *sem)
{
    MMLSEMAnalyzer *self = _self;
    self->builder->appendPan(self->builder, self->tick, self->channel, sem->value - 64);
}

static void visitDetune(void *_self, SEMDetune *sem)
{
    MMLSEMAnalyzer *self = _self;
    self->builder->appendDetune(self->builder, self->tick, self->channel, sem->value);
}

static void visitTempo(void *_self, SEMTempo *sem)
{
    MMLSEMAnalyzer *self = _self;
    self->builder->appendTempo(self->builder, self->tick, sem->tempo);
}

static int calcStep(MMLSEMAnalyzer *self, NoteLength *noteLength)
{
    if (-1 != noteLength->step) {
        return noteLength->step;
    }

    int length = -1 != noteLength->length ? noteLength->length : self->length;
    int step = self->timebase * 4 / length;
    int dotIncrement = step / 2;
    for (int i = 0; i < noteLength->dotCount; ++i) {
        step += dotIncrement;
        dotIncrement /= 2;
    }
    return step;
}

static int calcGatetime(MMLSEMAnalyzer *self, int step)
{
    int gatetime = step * self->gatetime.rate / 16
                 - self->timebase * 4 * self->gatetime.minus / 192;
    return MAX(0, gatetime);
}

static void visitNote(void *_self, SEMNote *sem)
{
    MMLSEMAnalyzer *self = _self;

    int noteNo = NoteTableGetNoteNo(self->noteTable, sem->baseNote, sem->accidental, self->octave);
    noteNo += self->transpose;

    if (!isValidRange(noteNo, 0, 127)) {
        appendError(self, sem, MMLParseErrorInvalidNoteNumber, NACStringFromInteger(noteNo), sem->noteString, NULL);
        return;
    }

    int step;
    if (self->tuplet) {
        step = calcStep(self, &self->tuplet->length) / self->tuplet->division;
    }
    else {
        step = calcStep(self, &sem->length);
    }

    int gatetime = calcGatetime(self, step);

    if (self->inChord) {
        int tick = self->tick + self->chord.offset;
        if (self->tie && processTie(self, step + self->chord.offset, self->channel, noteNo)) {
            ;
        }
        else {
            appendPendingNote(self, tick, self->channel, noteNo, gatetime, self->velocity.value);
        }
        self->chord.step = MAX(self->chord.step, step + self->chord.offset);
    }
    else {
        preprocessTie(self);

        if (self->tie && processTie(self, step, self->channel, noteNo)) {
            flushPreviousPendingNoteWithoutTie(self, self->tick);
        }
        else {
            flushPendingNote(self);
            appendPendingNote(self, self->tick, self->channel, noteNo, gatetime, self->velocity.value);
        }

        self->tie = false;
        self->tick += step;
    }
}

static void visitRest(void *_self, SEMRest *sem)
{
    MMLSEMAnalyzer *self = _self;

    int step;
    if (self->tuplet) {
        step = calcStep(self, &self->tuplet->length) / self->tuplet->division;
    }
    else {
        step = calcStep(self, &sem->length);
    }

    if (self->inChord) {
        self->chord.offset += step;
    }
    else {
        flushPendingNote(self);
        self->tie = false;
        self->tick += step;
    }
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
    MMLSEMAnalyzer *self = _self;
    self->tie = true;
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

    if (sem->direction) {
        int shift = '(' == sem->direction ? sem->value : -sem->value;
        shift *= self->velocity.absolute ? 1 : 8;
        shift *= self->velocityReverse ? -1 : 1;
        int velocity = self->velocity.value + shift;
        if (!isValidRange(velocity, 0, 127)) {
            appendError(self, sem, MMLParseErrorInvalidVelocity, NACStringFromInteger(velocity), NULL);
        }
        else {
            self->velocity.value = velocity;
        }
    }
    else {
        self->velocity.absolute = sem->absolute;
        self->velocity.value = self->velocity.absolute ? sem->value : sem->value * 8 + 7;
    }
}

static void visitTuplet(void *_self, SEMTuplet *sem)
{
    MMLSEMAnalyzer *self = _self;

    self->tuplet = sem;
    int tick = self->tick;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->tuplet = NULL;
    self->tick = tick + calcStep(self, &sem->length);
}

static void visitTrackChange(void *_self, SEMTrackChange *sem)
{
    MMLSEMAnalyzer *self = _self;
    flushPendingNote(self);
    self->sequenceLength = MAX(self->sequenceLength, self->tick);
    self->tick = 0;
}

static void visitRepeat(void *_self, SEMRepeat *sem)
{
    MMLSEMAnalyzer *self = _self;

    if (self->repeatContext) {
        NAStackPush(self->repeatContextStack, self->repeatContext);
    }
    
    self->repeatContext = RepeatContextCreate(sem);

    for (int i = 0; i < self->repeatContext->sem->times; ++i) {
        self->repeatContext->current = i + 1;

        NAIterator *iterator = NAArrayGetIterator(sem->node.children);
        while (iterator->hasNext(iterator)) {
            Node *node = iterator->next(iterator);
            node->accept(node, self);

            if (self->repeatContext->breaked) {
                break;
            }
        }
    }

    RepeatContextDestroy(self->repeatContext);
    self->repeatContext = NAStackPop(self->repeatContextStack);
}

static void visitRepeatBreak(void *_self, SEMRepeatBreak *sem)
{
    MMLSEMAnalyzer *self = _self;

    if (self->repeatContext->sem->times <= self->repeatContext->current) {
        self->repeatContext->breaked = true;
    }
}

static void visitChord(void *_self, SEMChord *sem)
{
    MMLSEMAnalyzer *self = _self;

    self->inChord = true;
    self->chord.offset = 0;
    self->chord.step = 0;

    if (self->tie) {
        preprocessTie(self);
    }
    else {
        flushPendingNote(self);
    }

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    flushPreviousPendingNoteWithoutTie(self, self->tick);

    self->tie = false;
    self->inChord = false;

    self->tick += self->chord.step;
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
    self->octave = 2;
    self->length = 4;
    self->gatetime.rate = 15;
    self->gatetime.minus = 0;
    self->velocity.value = 100;

    self->noteTable = NoteTableCreate(BaseNote_C, false, false, ModeMajor);
    self->pendingNotes = NAArrayCreate(4, NADescriptionAddress);

    return &self->analyzer;
}

static void appendPendingNote(MMLSEMAnalyzer *self, int tick, int channel, int noteNo, int gatetime, int velocity)
{
    Note *note = calloc(1, sizeof(Note));
    note->tick = tick;
    note->channel = channel;
    note->noteNo = noteNo;
    note->gatetime = gatetime;
    note->velocity = velocity;

    NAArrayAppend(self->pendingNotes, note);
}

static void flushPendingNote(MMLSEMAnalyzer *self)
{
    NAIterator *iterator = NAArrayGetIterator(self->pendingNotes);
    while (iterator->hasNext(iterator)) {
        Note *note = iterator->next(iterator);
        self->builder->appendNote(self->builder, note->tick, note->channel, note->noteNo, note->gatetime, note->velocity);
        free(note);
    }
    NAArrayRemoveAll(self->pendingNotes);
}

static bool processTie(MMLSEMAnalyzer *self, int step, int channel, int noteNo)
{
    bool ret = false;

    NAIterator *iterator = NAArrayGetIterator(self->pendingNotes);
    while (iterator->hasNext(iterator)) {
        Note *note = iterator->next(iterator);
        if (note->channel == channel && note->noteNo == noteNo) {
            note->gatetime += step;
            note->tied = true;
            ret = true;
        }
    }

    return ret;
}

static void preprocessTie(MMLSEMAnalyzer *self)
{
    NAIterator *iterator = NAArrayGetIterator(self->pendingNotes);
    while (iterator->hasNext(iterator)) {
        Note *note = iterator->next(iterator);
        note->tied = false;
    }
}

static void flushPreviousPendingNoteWithoutTie(MMLSEMAnalyzer *self, int tick)
{
    NAIterator *iterator = NAArrayGetIterator(self->pendingNotes);
    while (iterator->hasNext(iterator)) {
        Note *note = iterator->next(iterator);
        if (!note->tied && note->tick < tick) {
            self->builder->appendNote(self->builder, note->tick, note->channel, note->noteNo, note->gatetime, note->velocity);
            iterator->remove(iterator);
            free(note);
        }
    }
}

static RepeatContext *RepeatContextCreate(SEMRepeat *sem)
{
    RepeatContext *self = calloc(1, sizeof(RepeatContext));
    self->sem = sem;
    return self;
}

static void RepeatContextDestroy(RepeatContext *self)
{
    free(self);
}
