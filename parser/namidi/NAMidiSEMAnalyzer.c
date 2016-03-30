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
    NoteTable *noteTableForPercussion;
    struct {
        int step;
        int tick;
        int velocity;
        struct {
            bool absolute;
            int step;
            int value;
        } gatetime;
        int octave;
        bool percussion;
    } channels[16];

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
        Node *copyright;
    } definedNode;
} NAMidiSEMAnalyzer;

static State *StateCreate();
static State *StateCopy(State *self);
static void StateDestroy(State *self);
static int StateLength(State *self);

#define TICK(state) (state->channels[state->channel - 1].tick)
#define STEP(state) (state->channels[state->channel - 1].step)
#define GATETIME(state) (state->channels[state->channel - 1].gatetime)
#define VELOCITY(state) (state->channels[state->channel - 1].velocity)
#define OCTAVE(state) (state->channels[state->channel - 1].octave)
#define PERCUSSION(state) (state->channels[state->channel - 1].percussion)

#define FLUSH(state) TICK(state) += STEP(state); STEP(state) = 0

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

    FLUSH(self->state);
}

static void visitResolution(void *_self, SEMResolution *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    if (self->definedNode.resolution) {
        FileLocation *loc = &self->definedNode.resolution->location;
        appendError(self, sem, NAMidiParseErrorAlreadyDefinedWithResolution, loc->filepath, NACStringFromInteger(loc->line), NACStringFromInteger(loc->column), NULL);
        return;
    }

    FLUSH(self->state);

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

    FLUSH(self->state);

    self->builder->appendTitle(self->builder, 0, sem->title);
    self->definedNode.title = (Node *)sem;
}

static void visitCopyright(void *_self, SEMCopyright *sem)
{
    NAMidiSEMAnalyzer *self = _self;

    if (self->definedNode.copyright) {
        FileLocation *loc = &self->definedNode.copyright->location;
        appendError(self, sem, NAMidiParseErrorAlreadyDefinedWithCopyright, loc->filepath, NACStringFromInteger(loc->line), NACStringFromInteger(loc->column), NULL);
        return;
    }

    FLUSH(self->state);

    self->builder->appendCopyright(self->builder, 0, sem->text);
    self->definedNode.copyright = (Node *)sem;
}

static void visitTempo(void *_self, SEMTempo *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    self->builder->appendTempo(self->builder, TICK(self->state), sem->tempo);
}

static void visitTime(void *_self, SEMTime *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    self->builder->appendTimeSign(self->builder, TICK(self->state), sem->numerator, sem->denominator);
}

static void visitKey(void *_self, SEMKey *sem)
{
    NAMidiSEMAnalyzer *self = _self;

    FLUSH(self->state);

    MidiKeySign keysign = NoteTableGetMidiKeySign(sem->noteTable);
    self->builder->appendKey(self->builder, TICK(self->state), keysign.sf, keysign.mi);

    NoteTableRelease(self->state->noteTable);
    self->state->noteTable = NoteTableRetain(sem->noteTable);
}

static void visitPercussion(void *_self, SEMPercussion *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    PERCUSSION(self->state) = sem->on;
}

static void visitMarker(void *_self, SEMMarker *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    self->builder->appendMarker(self->builder, TICK(self->state), sem->text);
}

static void visitChannel(void *_self, SEMChannel *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    self->state->channel = sem->number;
}

static void visitVelocity(void *_self, SEMVelocity *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    VELOCITY(self->state) = sem->value;
}

static void visitGatetime(void *_self, SEMGatetime *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    GATETIME(self->state).absolute = sem->absolute;
    GATETIME(self->state).value = sem->value;
}

static void visitBank(void *_self, SEMBank *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    self->builder->appendBank(self->builder, TICK(self->state), self->state->channel, sem->bankNo);
}

static void visitProgram(void *_self, SEMProgram *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    self->builder->appendProgram(self->builder, TICK(self->state), self->state->channel, sem->programNo);
}

static void visitSynth(void *_self, SEMSynth *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    self->builder->appendSynth(self->builder, TICK(self->state), self->state->channel, sem->name);
}

static void visitVolume(void *_self, SEMVolume *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    self->builder->appendVolume(self->builder, TICK(self->state), self->state->channel, sem->value);
}

static void visitPan(void *_self, SEMPan *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    self->builder->appendPan(self->builder, TICK(self->state), self->state->channel, sem->value);
}

static void visitChorus(void *_self, SEMChorus *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    self->builder->appendChorus(self->builder, TICK(self->state), self->state->channel, sem->value);
}

static void visitReverb(void *_self, SEMReverb *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    self->builder->appendReverb(self->builder, TICK(self->state), self->state->channel, sem->value);
}

static void visitExpression(void *_self, SEMExpression *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    self->builder->appendExpression(self->builder, TICK(self->state), self->state->channel, sem->value);
}

static void visitPitch(void *_self, SEMPitch *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    self->builder->appendPitch(self->builder, TICK(self->state), self->state->channel, sem->value);
}

static void visitDetune(void *_self, SEMDetune *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    self->builder->appendDetune(self->builder, TICK(self->state), self->state->channel, sem->value);
}

static void visitPitchSense(void *_self, SEMPitchSense *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    self->builder->appendPitchSense(self->builder, TICK(self->state), self->state->channel, sem->value);
}

static void visitTranspose(void *_self, SEMTranspose *sem)
{
    NAMidiSEMAnalyzer *self = _self;
    FLUSH(self->state);
    self->state->transpose = sem->value;
}

static void visitStep(void *_self, SEMStep *sem)
{
    NAMidiSEMAnalyzer *self = _self;

    FLUSH(self->state);

    STEP(self->state) = sem->step;
    GATETIME(self->state).step = sem->step;
}

static void visitNote(void *_self, SEMNote *sem)
{
    NAMidiSEMAnalyzer *self = _self;

    int octave = SEMNOTE_OCTAVE_NONE != sem->octave ? sem->octave : OCTAVE(self->state);

    int noteNo;
    if (PERCUSSION(self->state)) {
        noteNo = NoteTableGetNoteNo(self->state->noteTableForPercussion, sem->baseNote, sem->accidental, octave);
    }
    else {
        noteNo = NoteTableGetNoteNo(self->state->noteTable, sem->baseNote, sem->accidental, octave);
        noteNo += self->state->transpose;
    }

    if (!isValidRange(noteNo, 0, 127)) {
        appendError(self, sem, NAMidiParseErrorInvalidNoteNumber, NACStringFromInteger(noteNo), sem->noteString, NULL);
        return;
    }

    OCTAVE(self->state) = octave;

    int gatetime = -1 != sem->gatetime ? sem->gatetime
                 : GATETIME(self->state).absolute ? GATETIME(self->state).value
                 : GATETIME(self->state).step + GATETIME(self->state).value;
    int velocity = -1 != sem->velocity ? sem->velocity : VELOCITY(self->state);

    self->builder->appendNote(self->builder, TICK(self->state), self->state->channel, noteNo, gatetime, velocity);
}

static SEMList *findPattern(NAMidiSEMAnalyzer *self, const char *identifier)
{
    char **ids = NACStringSplit(NACStringDuplicate(identifier), ":", NULL);
    NAMap *petternMap = self->state->patternMap;
    SEMList *list = NULL;
    while (*ids) {
        list = NAMapGet(petternMap, *ids);
        if (!list) {
            return NULL;
        }

        petternMap = list->patternMap;
        ++ids;
    }

    return list;
}

static void visitExpand(void *_self, SEMExpand *sem)
{
    NAMidiSEMAnalyzer *self = _self;

    SEMList *pattern = findPattern(self, sem->identifier);
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

    NASetAdd(self->state->expandingPatternList, pattern);
    pattern->node.accept(pattern, self);
    NASetRemove(self->state->expandingPatternList, pattern);

    FLUSH(self->state);

    State *local = self->state;
    self->state = NAStackPop(self->stateStack);
    for (int i = 0; i < 16; ++i) {
        self->state->channels[i].tick = local->channels[i].tick;
    }

    StateDestroy(local);
}

Analyzer *NAMidiSEMAnalyzerCreate(ParseContext *context)
{
    NAMidiSEMAnalyzer *self = calloc(1, sizeof(NAMidiSEMAnalyzer));
    
    self->visitor.visitList = visitList;
    self->visitor.visitResolution = visitResolution;
    self->visitor.visitTitle = visitTitle;
    self->visitor.visitCopyright = visitCopyright;
    self->visitor.visitTempo = visitTempo;
    self->visitor.visitTime = visitTime;
    self->visitor.visitKey = visitKey;
    self->visitor.visitPercussion = visitPercussion;
    self->visitor.visitMarker = visitMarker;
    self->visitor.visitChannel = visitChannel;
    self->visitor.visitVelocity = visitVelocity;
    self->visitor.visitGatetime = visitGatetime;
    self->visitor.visitBank = visitBank;
    self->visitor.visitProgram = visitProgram;
    self->visitor.visitSynth = visitSynth;
    self->visitor.visitVolume = visitVolume;
    self->visitor.visitPan = visitPan;
    self->visitor.visitChorus = visitChorus;
    self->visitor.visitReverb = visitReverb;
    self->visitor.visitExpression = visitExpression;
    self->visitor.visitPitch = visitPitch;
    self->visitor.visitDetune = visitDetune;
    self->visitor.visitPitchSense = visitPitchSense;
    self->visitor.visitTranspose = visitTranspose;
    self->visitor.visitStep = visitStep;
    self->visitor.visitNote = visitNote;
    self->visitor.visitExpand = visitExpand;

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
    self->noteTableForPercussion = NoteTableRetain(self->noteTable);

    self->resolution = 480;
    self->channel = 1;

    for (int i = 0; i < 16; ++i) {
        self->channels[i].gatetime.absolute = false;
        self->channels[i].gatetime.value = 0;
        self->channels[i].velocity = 100;
        self->channels[i].octave = 2;
    }

    self->patternMap = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    self->expandingPatternList = NASetCreate(NAHashAddress, NADescriptionAddress);

    return self;
}

static State *StateCopy(State *self)
{
    State *copy = calloc(1, sizeof(State));
    memcpy(copy, self, sizeof(State));

    NoteTableRetain(copy->noteTable);
   
    copy->patternMap = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    NAIterator *iterator = NAMapGetIterator(self->patternMap);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        NAMapPut(copy->patternMap, entry->key, entry->value);
    }

    copy->copy = true;
    return copy;
}

static void StateDestroy(State *self)
{
    NoteTableRelease(self->noteTable);

    if (!self->copy) {
        NASetDestroy(self->expandingPatternList);
        NoteTableRelease(self->noteTableForPercussion);
    }

    NAMapDestroy(self->patternMap);
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
