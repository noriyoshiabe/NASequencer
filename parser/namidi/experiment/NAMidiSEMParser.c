#include "NAMidiSEMParser.h"
#include "NAMidiSEM.h"
#include "Parser.h"
#include "SequenceBuilder.h"
#include "NAMap.h"
#include "NASet.h"
#include "NAStack.h"
#include "NACString.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>

#define __Trace__ printf("%s - %s\n", __FILE__, __func__);

#define isValidRange(v, from, to) (from <= v && v <= to)

typedef struct _Context {
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
} Context;

struct _NAMidiSEMParser {
    SEMVisitor visitor;
    void *parser;
    SequenceBuilder *builder;
    Context *context;
    NAStack *contextStack;
};

static Context *ContextCreate();
static Context *ContextCopy(Context *self);
static void ContextDestroy(Context *self);
static int ContextTick(Context *self);
static int ContextLength(Context *self);

#define TICK(ctx) (ctx->channels[ctx->channel].tick)

static void visitList(void *self, SEMList *sem);
static void visitResolution(void *self, SEMResolution *sem);
static void visitTitle(void *self, SEMTitle *sem);
static void visitTempo(void *self, SEMTempo *sem);
static void visitTime(void *self, SEMTime *sem);
static void visitKey(void *self, SEMKey *sem);
static void visitMarker(void *self, SEMMarker *sem);
static void visitChannel(void *self, SEMChannel *sem);
static void visitVoice(void *self, SEMVoice *sem);
static void visitSynth(void *self, SEMSynth *sem);
static void visitVolume(void *self, SEMVolume *sem);
static void visitPan(void *self, SEMPan *sem);
static void visitChorus(void *self, SEMChorus *sem);
static void visitReverb(void *self, SEMReverb *sem);
static void visitTranspose(void *self, SEMTranspose *sem);
static void visitRest(void *self, SEMRest *sem);
static void visitNote(void *self, SEMNote *sem);
static void visitPattern(void *self, SEMPattern *sem);
static void visitContext(void *self, SEMContext *sem);

NAMidiSEMParser *NAMidiSEMParserCreate(void *parser, void *builder)
{
    NAMidiSEMParser *self = calloc(1, sizeof(NAMidiSEMParser));
    
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

    self->parser = parser;
    self->builder = builder;

    self->context = ContextCreate();
    self->contextStack = NAStackCreate(4);

    return self;
}

void NAMidiSEMParserDestroy(NAMidiSEMParser *self)
{
    NAStackDestroy(self->contextStack);
    ContextDestroy(self->context);
    free(self);
}

void *NAMidiSEMParserBuildSequence(NAMidiSEMParser *self, Node *node)
{
    node->accept(node, self);
    self->builder->setLength(self->builder, ContextLength(self->context));
    return self->builder->build(self->builder);
}

static void visitList(void *_self, SEMList *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;
    NAIterator *iterator;
   
    iterator = NAMapGetIterator(sem->patternMap);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        NAMapPut(self->context->patternMap, entry->key, entry->value);
    }

    iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitResolution(void *_self, SEMResolution *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;
    self->builder->setResolution(self->builder, sem->resolution);
}

static void visitTitle(void *_self, SEMTitle *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;
    self->builder->setTitle(self->builder, sem->title);
}

static void visitTempo(void *_self, SEMTempo *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;
    self->builder->appendTempo(self->builder, TICK(self->context), sem->tempo);
}

static void visitTime(void *_self, SEMTime *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;
    self->builder->appendTimeSign(self->builder, TICK(self->context), sem->numerator, sem->denominator);
}

static void visitKey(void *_self, SEMKey *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;
    MidiKeySign keysign = NoteTableGetMidiKeySign(sem->noteTable);
    self->builder->appendKey(self->builder, TICK(self->context), keysign.sf, keysign.mi);

    NoteTableRelease(self->context->noteTable);
    self->context->noteTable = NoteTableRetain(sem->noteTable);
}

static void visitMarker(void *_self, SEMMarker *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;
    self->builder->appendMarker(self->builder, TICK(self->context), sem->text);
}

static void visitChannel(void *_self, SEMChannel *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;
    self->context->channel = sem->number;
}

static void visitVoice(void *_self, SEMVoice *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;
    self->builder->appendVoice(self->builder, TICK(self->context), self->context->channel, sem->msb, sem->lsb, sem->programNo);
}

static void visitSynth(void *_self, SEMSynth *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;
    self->builder->appendSynth(self->builder, TICK(self->context), self->context->channel, sem->name);
}

static void visitVolume(void *_self, SEMVolume *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;
    self->builder->appendVolume(self->builder, TICK(self->context), self->context->channel, sem->value);
}

static void visitPan(void *_self, SEMPan *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;
    self->builder->appendPan(self->builder, TICK(self->context), self->context->channel, sem->value);
}

static void visitChorus(void *_self, SEMChorus *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;
    self->builder->appendChorus(self->builder, TICK(self->context), self->context->channel, sem->value);
}

static void visitReverb(void *_self, SEMReverb *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;
    self->builder->appendReverb(self->builder, TICK(self->context), self->context->channel, sem->value);
}

static void visitTranspose(void *_self, SEMTranspose *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;
    self->context->transpose = sem->value;
}

static void visitRest(void *_self, SEMRest *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;
    TICK(self->context) += sem->step;
}

static void visitNote(void *_self, SEMNote *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;

    int channel = self->context->channel;
    int octave = SEMNOTE_OCTAVE_NONE != sem->octave ? sem->octave : self->context->channels[channel].octave;
    self->context->channels[channel].octave = octave;

    int noteNo = NoteTableGetNoteNo(self->context->noteTable, sem->baseNote, sem->accidental, octave);
    noteNo += self->context->transpose;
    if (!isValidRange(noteNo, 0, 127)) {
        ParserError(self->parser, &sem->node.location, NAMidiParseErrorInvalidNoteNumber, NACStringFromInteger(noteNo), sem->noteString, NULL);
        return;
    }

    int step = -1 != sem->step ? sem->step : 0;

    int gatetime = -1 != sem->gatetime ? sem->gatetime : self->context->channels[channel].gatetime;
    self->context->channels[channel].gatetime = gatetime;

    int velocity = -1 != sem->velocity ? sem->velocity : self->context->channels[channel].velocity;
    self->context->channels[channel].velocity = velocity;

    self->builder->appendNote(self->builder, TICK(self->context), channel, noteNo, gatetime, velocity);
    TICK(self->context) += step;
}

static void visitPattern(void *_self, SEMPattern *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;

    SEMList *pattern = NAMapGet(self->context->patternMap, sem->identifier);
    if (!pattern) {
        ParserError(self->parser, &sem->node.location, NAMidiParseErrorPatternMissing, sem->identifier, NULL);
        return;
    }

    if (NASetContains(self->context->expandingPatternList, pattern)) {
        ParserError(self->parser, &sem->node.location, NAMidiParseErrorCircularPatternReference, sem->identifier, NULL);
        return;
    }

    NAStackPush(self->contextStack, self->context);
    self->context = ContextCopy(self->context);

    NAIterator *iterator = NAArrayGetIterator(sem->ctxIdList);
    while (iterator->hasNext(iterator)) {
        NASetAdd(self->context->ctxIdList, iterator->next(iterator));
    }

    NASetAdd(self->context->expandingPatternList, pattern);
    pattern->node.accept(pattern, self);
    NASetRemove(self->context->expandingPatternList, pattern);

    Context *local = self->context;
    self->context = NAStackPop(self->contextStack);
    for (int i = 0; i < 16; ++i) {
        self->context->channels[i].tick = local->channels[i].tick;
    }

    ContextDestroy(local);
}

static void visitContext(void *_self, SEMContext *sem)
{ __Trace__
    NAMidiSEMParser *self = _self;

    NAIterator *iterator = NAArrayGetIterator(sem->ctxIdList);
    while (iterator->hasNext(iterator)) {
        if (NASetContains(self->context->ctxIdList, iterator->next(iterator))) {
            sem->list->node.accept(sem->list, self);
            return;
        }
    }
}

static Context *ContextCreate()
{
    Context *self = calloc(1, sizeof(Context));

    self->noteTable = NoteTableCreate(BaseNote_C, false, false, ModeMajor);

    for (int i = 0; i < 16; ++i) {
        self->channels[i].gatetime = 240;
        self->channels[i].velocity = 100;
        self->channels[i].octave = 2;
    }

    self->channel = 1;
    self->patternMap = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    self->ctxIdList = NASetCreate(NAHashCString, NADescriptionCString);
    self->expandingPatternList = NASetCreate(NAHashAddress, NADescriptionAddress);

    return self;
}

static Context *ContextCopy(Context *self)
{
    NAIterator *iterator;

    Context *copy = calloc(1, sizeof(Context));
    memcpy(copy, self, sizeof(Context));

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

static void ContextDestroy(Context *self)
{
    NoteTableRelease(self->noteTable);

    if (!self->copy) {
        NASetDestroy(self->expandingPatternList);
    }

    NAMapDestroy(self->patternMap);
    NASetDestroy(self->ctxIdList);

    free(self);
}

static int ContextLength(Context *self)
{
    int length = 0;
    for (int i = 0; i < 16; ++i) {
        length = MAX(length, self->channels[i].tick);
    }
    return length;
}
