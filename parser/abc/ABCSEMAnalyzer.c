#include "ABCSEMAnalyzer.h"
#include "ABCSEM.h"
#include "ABCParser.h"
#include "NACInteger.h"
#include "NACString.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/param.h>

#define RESOLUTION 480
#define appendError(self, sem, ...) self->context->appendError(self->context, &sem->node.location, __VA_ARGS__)
#define isValidRange(v, from, to) (from <= v && v <= to)

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

typedef struct _VoiceContext {
    int channel;
    int tick;
    int unitNoteLength;
    int transpose;
    int octave;
    int velocity;
    bool accent;
} VoiceContext;

static VoiceContext *VoiceContextCreate();
static void VoiceContextDestroy(VoiceContext *self);

typedef struct {
    int tick;
    int channel;
    int noteNo;
    int gatetime;
    int velocity;
} Note;

static Note *NoteCreate();
static void NoteDestroy(Note *self);

typedef struct _ABCSEMAnalyzer {
    SEMVisitor visitor;
    Analyzer analyzer;

    ParseContext *context;
    SequenceBuilder *builder;

    NAMap *repeatMap;
    NAMap *voiceMap;

    RepeatContext *repeat;
    VoiceContext *voice;

    SEMTune *tune;
    SEMKey *key;

    struct {
        SEMMeter *meter;
        SEMTempo *tempo;
        int unitNoteLength;
        int tick;
    } file;

    struct {
        struct {
            SEMMeter *sem;
            int tick;
        } meter;
        struct {
            SEMTempo *sem;
            int tick;
        } tempo;
    } pending;

    NAArray *pendingNotes;
} ABCSEMAnalyzer;

static int VoiceIdComparator(const void *_id1, const void *_id2);
#define inFileFeader(self) (NULL == self->tune)
static void flushPendingNotes(ABCSEMAnalyzer *self);

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
    NAMapTraverseValue(self->voiceMap, VoiceContextDestroy);
    NAMapDestroy(self->voiceMap);
    NAArrayTraverse(self->pendingNotes, NoteDestroy);
    NAArrayDestroy(self->pendingNotes);
    free(self);
}

static void visitFile(void *_self, SEMFile *sem)
{
    ABCSEMAnalyzer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void setFileContext(ABCSEMAnalyzer *self)
{
    if (self->file.meter) {
        self->pending.meter.sem = self->file.meter;
        self->pending.meter.tick = 0;
    }

    if (self->file.tempo) {
        self->pending.tempo.sem = self->file.tempo;
        self->pending.tempo.tick = 0;
    }

    NAIterator *iterator = NAMapGetIterator(self->voiceMap);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        VoiceContext *voice = entry->value;
        voice->unitNoteLength = self->file.unitNoteLength;
        voice->tick = self->file.tick;
    }
}

static void processPendingEvents(ABCSEMAnalyzer *self)
{
    if (self->pending.meter.sem) {
        SEMMeter *_sem = self->pending.meter.sem;
        int tick = self->pending.meter.tick;
        self->pending.meter.sem = NULL;
        self->builder->appendTimeSign(self->builder, tick, _sem->numerator, _sem->denominator);
    }

    if (self->pending.tempo.sem) {
        SEMTempo *_sem = self->pending.tempo.sem;
        int tick = self->pending.tempo.tick;
        self->pending.tempo.sem = NULL;
        self->builder->appendTempo(self->builder, tick, _sem->tempo);
    }
}

static void postProcessTune(ABCSEMAnalyzer *self)
{
    processPendingEvents(self);

    int tick = 0;

    NAIterator *iterator = NAMapGetIterator(self->voiceMap);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        VoiceContext *voice = entry->value;
        tick = MAX(tick, voice->tick);
    }

    self->builder->setLength(self->builder, tick);
    self->file.tick = tick + RESOLUTION * 4;
}

static void visitTune(void *_self, SEMTune *sem)
{
    ABCSEMAnalyzer *self = _self;
    
    self->tune = sem;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    int count = NAMapCount(sem->voiceMap);
    char **voiceIds = NAMapGetKeys(sem->voiceMap, alloca(sizeof(char *) * count));
    qsort(voiceIds, count, sizeof(char *), VoiceIdComparator);
    for (int i = 0; i < count; ++i) {
        SEMVoice *voice = NAMapGet(sem->voiceMap, voiceIds[i]);

        VoiceContext *context = VoiceContextCreate();
        context->channel = i + 1;
        context->transpose = voice->transpose;
        context->octave = voice->octave;
        context->velocity = 100;
        NAMapPut(self->voiceMap, voiceIds[i], context);
    }

    setFileContext(self);

    int length = sem->partSequence ? strlen(sem->partSequence) + 1 : 1;
    char *partSequence = alloca(length + 1);
    sprintf(partSequence, "#%s", sem->partSequence);

    for (int i = 0; i < length; ++i) {
        Node *part = NAMapGet(sem->partMap, NACStringFromChar(partSequence[i]));
        part->accept(part, self);
    }

    postProcessTune(self);
}

static void visitKey(void *_self, SEMKey *sem)
{
    ABCSEMAnalyzer *self = _self;
    self->key = sem;

    MidiKeySign keysign = NoteTableGetMidiKeySign(sem->noteTable);
    self->builder->appendKey(self->builder, self->voice->tick, keysign.sf, keysign.mi);
}

static void visitMeter(void *_self, SEMMeter *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (inFileFeader(self)) {
        self->file.meter = sem;
    }
    else {
        if (!self->pending.meter.sem) {
            self->pending.meter.sem = sem;
            self->pending.meter.tick = self->voice->tick;
        }
        else {
            if (self->pending.meter.tick == self->voice->tick) {
                self->pending.meter.sem = sem;
            }
            else {
                SEMMeter *_sem = self->pending.meter.sem;
                self->pending.meter.sem = NULL;
                self->builder->appendTimeSign(self->builder, self->voice->tick, _sem->numerator, _sem->denominator);
            }
        }
    }
}

static void visitUnitNoteLength(void *_self, SEMUnitNoteLength *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (inFileFeader(self)) {
        self->file.unitNoteLength = sem->length;
    }
    else {
        self->voice->unitNoteLength = sem->length;
    }
}

static void visitTempo(void *_self, SEMTempo *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (inFileFeader(self)) {
        self->file.tempo = sem;
    }
    else {
        if (!self->pending.tempo.sem) {
            self->pending.tempo.sem = sem;
            self->pending.tempo.tick = self->voice->tick;
        }
        else {
            if (self->pending.tempo.tick == self->voice->tick) {
                self->pending.tempo.sem = sem;
            }
            else {
                SEMTempo *_sem = self->pending.tempo.sem;
                self->pending.tempo.sem = NULL;
                self->builder->appendTempo(self->builder, self->voice->tick, _sem->tempo);
            }
        }
    }
}

static void visitPart(void *_self, SEMPart *sem)
{
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
    ABCSEMAnalyzer *self = _self;

    self->voice = NAMapGet(self->voiceMap, sem->voiceId);

    self->repeat = NAMapGet(self->repeatMap, sem);
    if (!self->repeat) {
        self->repeat = RepeatContextCreate();
        NAMapPut(self->repeatMap, sem, self->repeat);
    }

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

    flushPendingNotes(self);
}

static void visitVoice(void *_self, SEMVoice *sem)
{
    ABCSEMAnalyzer *self = _self;
    self->voice->transpose = sem->transpose;
    self->voice->octave = sem->octave;
}

static void visitDecoration(void *_self, SEMDecoration *sem)
{
    ABCSEMAnalyzer *self = _self;
    
    switch (sem->type) {
    case Accent:
    case Emphasis:
        self->voice->accent = true;
        break;
    case PianoPianissimo:
        self->voice->velocity = 30;
        break;
    case Pianissimo:
        self->voice->velocity = 50;
        break;
    case Piano:
        self->voice->velocity = 70;
        break;
    case MezzoPiano:
        self->voice->velocity = 90;
        break;
    case MezzoForte:
        self->voice->velocity = 100;
        break;
    case Forte:
        self->voice->velocity = 110;
        break;
    case Foruthisimo:
        self->voice->velocity = 120;
        break;
    case ForteForuthisimo:
        self->voice->velocity = 127;
        break;
    }
}

static void flushPendingNotes(ABCSEMAnalyzer *self)
{
    NAIterator *iterator = NAArrayGetIterator(self->pendingNotes);
    while (iterator->hasNext(iterator)) {
        Note *note = iterator->next(iterator);
        self->builder->appendNote(self->builder, note->tick, note->channel, note->noteNo, note->gatetime, note->velocity);
        NoteDestroy(note);
    }

    NAArrayRemoveAll(self->pendingNotes);
}

static void visitNote(void *_self, SEMNote *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (RepeatStateInitial != self->repeat->state) {
        return;
    }

#if 1
    printf("[REPEAT TEST] baseNote=%s\n", BaseNote2String(sem->baseNote));
#endif

    // TODO tuplet
    int division = sem->length.divider;
    int multiplier = sem->length.multiplier;

    if (0 != self->voice->unitNoteLength * multiplier % division) {
        float result = (float)(self->voice->unitNoteLength * multiplier) / (float)division;
        appendError(self, sem, ABCParseErrorInvalidCaluculatedNoteLength, NACStringFromFloat(result, 2), NACStringFromInteger(RESOLUTION), NULL);
        return;
    }

    int step = self->voice->unitNoteLength * multiplier / division;

    int octave = self->key->octave + self->voice->octave;
    int transpose = self->key->transpose + self->voice->transpose;

    int noteNo = NoteTableGetNoteNo(self->key->noteTable, sem->baseNote, sem->accidental, octave + sem->octave);
    noteNo += transpose;

    if (!isValidRange(noteNo, 0, 127)) {
        appendError(self, sem, ABCParseErrorInvalidNoteNumber, NACStringFromInteger(noteNo),
                NACStringFromInteger(octave), NACStringFromInteger(transpose), sem->noteString, NULL);
    }
    else {
        Note *note = NoteCreate();
        note->tick = self->voice->tick;
        note->channel = self->voice->channel;
        note->noteNo = noteNo;
        note->gatetime = step;
        note->velocity = self->voice->velocity;

        NAArrayAppend(self->pendingNotes, note);
    }

    // TODO chord

    self->voice->tick += step;
}

static void visitBrokenRhythm(void *_self, SEMBrokenRhythm *sem)
{
    ABCSEMAnalyzer *self = _self;
}

static void visitRest(void *_self, SEMRest *sem)
{
    ABCSEMAnalyzer *self = _self;
}

static void visitRepeat(void *_self, SEMRepeat *sem)
{
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
    ABCSEMAnalyzer *self = _self;
}

static void visitTie(void *_self, SEMTie *sem)
{
    ABCSEMAnalyzer *self = _self;
}

static void visitGraceNote(void *_self, SEMGraceNote *sem)
{
    ABCSEMAnalyzer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitTuplet(void *_self, SEMTuplet *sem)
{
    ABCSEMAnalyzer *self = _self;
}

static void visitChord(void *_self, SEMChord *sem)
{
    ABCSEMAnalyzer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitOverlay(void *_self, SEMOverlay *sem)
{
    ABCSEMAnalyzer *self = _self;
}

static void visitMidiVoice(void *_self, SEMMidiVoice *sem)
{
    ABCSEMAnalyzer *self = _self;
}

static void visitPropagateAccidental(void *_self, SEMPropagateAccidental *sem)
{
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
    self->builder = context->builder;

    self->repeatMap = NAMapCreate(NAHashAddress, NADescriptionAddress, NADescriptionAddress);
    self->voiceMap = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    self->pendingNotes = NAArrayCreate(4, NADescriptionAddress);

    self->file.unitNoteLength = RESOLUTION / 2;

    return &self->analyzer;
}


static int VoiceIdComparator(const void *_id1, const void *_id2)
{
    const char **id1 = (const char **)_id1;
    const char **id2 = (const char **)_id2;

    return strcmp(*id1, *id2);
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
    free(self);
}

static VoiceContext *VoiceContextCreate()
{
    return calloc(1, sizeof(VoiceContext));
}

static void VoiceContextDestroy(VoiceContext *self)
{
    free(self);
}

static Note *NoteCreate()
{
    return calloc(1, sizeof(Note));
}

static void NoteDestroy(Note *self)
{
    free(self);
}
