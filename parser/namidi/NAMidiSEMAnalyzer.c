#include "NAMidiSEMAnalyzer.h"
#include "NAMidiParser.h"
#include "NAMidiSEM.h"
#include "NAMap.h"
#include "NASet.h"
#include "NAStack.h"
#include "NACString.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>

#define appendError(self, sem, ...) self->context->appendError(self->context, &sem->node.location, __VA_ARGS__)
#define isValidRange(v, from, to) (from <= v && v <= to)

typedef struct _State {
    int resolution;
    int channel;
    int transpose;
    NoteTable *noteTable;
    struct {
        int tick;
        int gatetime;
        int velocity;
        int octave;
    } channels[16];

    NASet *ctxIdList;

    NAMap *patternMap;
    NASet *expandingPatternList;

    bool copy;
} State;

typedef struct _NAMidiSEMAnalyzer {
    SEMVisitor visitor;
    Analyzer analyzer;
    ParseContext *context;
    SequenceBuilder *builder;
    State *state;
    NAStack *stateStack;

    struct {
        Node *resolution;
        Node *title;
    } definedNode;
} NAMidiSEMAnalyzer;

static State *StateCreate();
static State *StateCopy(State *self);
static void StateDestroy(State *self);
static int StateTick(State *self);
static int StateLength(State *self);

#define TICK(state) (state->channels[state->channel].tick)

static void destroy(void *_self)
{
    NAMidiSEMAnalyzer *self = _self;
    NAStackDestroy(self->stateStack);
    StateDestroy(self->state);
    free(self);
}

static Node *process(void *_self, Node *node)
{
    NAMidiSEMAnalyzer *self = _self;
    node->accept(node, self);
    self->builder->setLength(self->builder, StateLength(self->state));
    return NULL;
}

static void visitList(void *_self, SEMList *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    NAIterator *iterator;
   
    iterator = NAMapGetIterator(sem->patternMap);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        NAMapPut(self->state->patternMap, entry->key, entry->value);
    }

    iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitResolution(void *_self, SEMResolution *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    if (self->definedNode.resolution) {
        FileLocation *loc = &self->definedNode.resolution->location;
        appendError(self, sem, NAMidiParseErrorAlreadyDefinedWithResolution, loc->filepath, NACStringFromInteger(loc->line), NACStringFromInteger(loc->column), NULL);
        return;
    }

    self->state->resolution = sem->resolution;
    self->builder->setResolution(self->builder, sem->resolution);
    self->definedNode.resolution = (Node *)sem;
}

static void visitTitle(void *_self, SEMTitle *sem)
{
    NAMidiSEMAnalyzer *self = _self;

    if (self->definedNode.title) {
        FileLocation *loc = &self->definedNode.title->location;
        appendError(self, sem, NAMidiParseErrorAlreadyDefinedWithTitle, loc->filepath, NACStringFromInteger(loc->line), NACStringFromInteger(loc->column), NULL);
        return;
    }

    self->builder->setTitle(self->builder, sem->title);
    self->definedNode.title = (Node *)sem;
}

static void visitTempo(void *_self, SEMTempo *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    self->builder->appendTempo(self->builder, TICK(self->state), sem->tempo);
}

static void visitTime(void *_self, SEMTime *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    self->builder->appendTimeSign(self->builder, TICK(self->state), sem->numerator, sem->denominator);
}

static void visitKey(void *_self, SEMKey *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    MidiKeySign keysign = NoteTableGetMidiKeySign(sem->noteTable);
    self->builder->appendKey(self->builder, TICK(self->state), keysign.sf, keysign.mi);

    NoteTableRelease(self->state->noteTable);
    self->state->noteTable = NoteTableRetain(sem->noteTable);
}

static void visitMarker(void *_self, SEMMarker *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    self->builder->appendMarker(self->builder, TICK(self->state), sem->text);
}

static void visitChannel(void *_self, SEMChannel *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    self->state->channel = sem->number;
}

static void visitVoice(void *_self, SEMVoice *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    self->builder->appendVoice(self->builder, TICK(self->state), self->state->channel, sem->msb, sem->lsb, sem->programNo);
}

static void visitSynth(void *_self, SEMSynth *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    self->builder->appendSynth(self->builder, TICK(self->state), self->state->channel, sem->name);
}

static void visitVolume(void *_self, SEMVolume *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    self->builder->appendVolume(self->builder, TICK(self->state), self->state->channel, sem->value);
}

static void visitPan(void *_self, SEMPan *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    self->builder->appendPan(self->builder, TICK(self->state), self->state->channel, sem->value);
}

static void visitChorus(void *_self, SEMChorus *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    self->builder->appendChorus(self->builder, TICK(self->state), self->state->channel, sem->value);
}

static void visitReverb(void *_self, SEMReverb *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    self->builder->appendReverb(self->builder, TICK(self->state), self->state->channel, sem->value);
}

static void visitTranspose(void *_self, SEMTranspose *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    self->state->transpose = sem->value;
}

static void visitRest(void *_self, SEMRest *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    TICK(self->state) += sem->step;
}

static void visitNote(void *_self, SEMNote *sem)
{
    NAMidiSEMAnalyzer *self = _self;

    int channel = self->state->channel;
    int octave = SEMNOTE_OCTAVE_NONE != sem->octave ? sem->octave : self->state->channels[channel].octave;
    self->state->channels[channel].octave = octave;

    int noteNo = NoteTableGetNoteNo(self->state->noteTable, sem->baseNote, sem->accidental, octave);
    noteNo += self->state->transpose;
    if (!isValidRange(noteNo, 0, 127)) {
        appendError(self, sem, NAMidiParseErrorInvalidNoteNumber, NACStringFromInteger(noteNo), sem->noteString, NULL);
        return;
    }

    int step = -1 != sem->step ? sem->step : 0;

    int gatetime = -1 != sem->gatetime ? sem->gatetime
                 : -1 != self->state->channels[channel].gatetime ? self->state->channels[channel].gatetime
                 : self->state->resolution / 2;
    self->state->channels[channel].gatetime = gatetime;

    int velocity = -1 != sem->velocity ? sem->velocity : self->state->channels[channel].velocity;
    self->state->channels[channel].velocity = velocity;

    self->builder->appendNote(self->builder, TICK(self->state), channel, noteNo, gatetime, velocity);
    TICK(self->state) += step;
}

static void visitPattern(void *_self, SEMPattern *sem)
{
    NAMidiSEMAnalyzer *self = _self;

    SEMList *pattern = NAMapGet(self->state->patternMap, sem->identifier);
    if (!pattern) {
        appendError(self, sem, NAMidiParseErrorPatternMissing, sem->identifier, NULL);
        return;
    }

    if (NASetContains(self->state->expandingPatternList, pattern)) {
        appendError(self, sem, NAMidiParseErrorCircularPatternReference, sem->identifier, NULL);
        return;
    }

    NAStackPush(self->stateStack, self->state);
    self->state = StateCopy(self->state);

    NAIterator *iterator = NAArrayGetIterator(sem->ctxIdList);
    while (iterator->hasNext(iterator)) {
        NASetAdd(self->state->ctxIdList, iterator->next(iterator));
    }

    NASetAdd(self->state->expandingPatternList, pattern);
    pattern->node.accept(pattern, self);
    NASetRemove(self->state->expandingPatternList, pattern);

    State *local = self->state;
    self->state = NAStackPop(self->stateStack);
    for (int i = 0; i < 16; ++i) {
        self->state->channels[i].tick = local->channels[i].tick;
    }

    StateDestroy(local);
}

static void visitContext(void *_self, SEMContext *sem)
{
    NAMidiSEMAnalyzer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(sem->ctxIdList);
    while (iterator->hasNext(iterator)) {
        if (NASetContains(self->state->ctxIdList, iterator->next(iterator))) {
            sem->list->node.accept(sem->list, self);
            return;
        }
    }
}

Analyzer *NAMidiSEMAnalyzerCreate(ParseContext *context)
{
    NAMidiSEMAnalyzer *self = calloc(1, sizeof(NAMidiSEMAnalyzer));
    
    self->visitor.visitList = visitList;
    self->visitor.visitResolution = visitResolution;
    self->visitor.visitTitle = visitTitle;
    self->visitor.visitTempo = visitTempo;
    self->visitor.visitTime = visitTime;
    self->visitor.visitKey = visitKey;
    self->visitor.visitMarker = visitMarker;
    self->visitor.visitChannel = visitChannel;
    self->visitor.visitVoice = visitVoice;
    self->visitor.visitSynth = visitSynth;
    self->visitor.visitVolume = visitVolume;
    self->visitor.visitPan = visitPan;
    self->visitor.visitChorus = visitChorus;
    self->visitor.visitReverb = visitReverb;
    self->visitor.visitTranspose = visitTranspose;
    self->visitor.visitRest = visitRest;
    self->visitor.visitNote = visitNote;
    self->visitor.visitPattern = visitPattern;
    self->visitor.visitContext = visitContext;

    self->analyzer.process = process;
    self->analyzer.destroy = destroy;
    self->analyzer.self = self;

    self->context = context;
    self->builder = context->builder;

    self->state = StateCreate();
    self->stateStack = NAStackCreate(4);

    return &self->analyzer;
}

static State *StateCreate()
{
    State *self = calloc(1, sizeof(State));

    self->noteTable = NoteTableCreate(BaseNote_C, false, false, ModeMajor);

    self->resolution = 480;
    self->channel = 1;

    for (int i = 0; i < 16; ++i) {
        self->channels[i].gatetime = -1;
        self->channels[i].velocity = 100;
        self->channels[i].octave = 2;
    }

    self->patternMap = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    self->ctxIdList = NASetCreate(NAHashCString, NADescriptionCString);
    self->expandingPatternList = NASetCreate(NAHashAddress, NADescriptionAddress);

    return self;
}

static State *StateCopy(State *self)
{
    NAIterator *iterator;

    State *copy = calloc(1, sizeof(State));
    memcpy(copy, self, sizeof(State));

    NoteTableRetain(copy->noteTable);
   
    copy->patternMap = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    iterator = NAMapGetIterator(self->patternMap);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        NAMapPut(copy->patternMap, entry->key, entry->value);
    }

    copy->ctxIdList = NASetCreate(NAHashCString, NADescriptionCString);
    iterator = NASetGetIterator(self->ctxIdList);
    while (iterator->hasNext(iterator)) {
        NASetAdd(copy->ctxIdList, iterator->next(iterator));
    }

    copy->copy = true;
    return copy;
}

static void StateDestroy(State *self)
{
    NoteTableRelease(self->noteTable);

    if (!self->copy) {
        NASetDestroy(self->expandingPatternList);
    }

    NAMapDestroy(self->patternMap);
    NASetDestroy(self->ctxIdList);

    free(self);
}

static int StateLength(State *self)
{
    int length = 0;
    for (int i = 0; i < 16; ++i) {
        length = MAX(length, self->channels[i].tick);
    }
    return length;
}
