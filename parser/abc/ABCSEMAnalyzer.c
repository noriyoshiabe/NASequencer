#include "ABCSEMAnalyzer.h"
#include "ABCSEM.h"
#include "ABCParser.h"
#include "NACInteger.h"
#include "NACString.h"
#include "NAStack.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/param.h>

#define RESOLUTION 480
#define appendError(self, sem, ...) self->context->appendError(self->context, &((Node *)sem)->location, __VA_ARGS__)
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

typedef struct {
    int division;
    int time;
    int count;
} Tuplet;

#define TupletCreate() calloc(1, sizeof(Tuplet))
#define TupletDestroy  free

typedef struct _VoiceContext {
    int channel;
    int tick;
    int unitNoteLength;
    int transpose;
    int octave;
    int velocity;

    bool accent;
    bool tie;

    NAArray *pendingNotes;

    struct {
        struct {
            int multiplier;
            int divider;
        } prev;
        struct {
            int multiplier;
            int divider;
        } next;
    } brokenRhythm;

    Tuplet *tuplet;
    NAStack *tupletStack;

    bool inChord;
    bool inGraceNote;
    int graceNoteLength;

    int lastBarTick;
    int preIncrementTick;

    struct {
        bool untilBar;
        bool allOctave;
        struct {
            Accidental accidental;
            int octave;
        } notes[BaseNoteCount];
    } accidental;
} VoiceContext;

static VoiceContext *VoiceContextCreate();
static void VoiceContextDestroy(VoiceContext *self);

typedef struct {
    int step;
    int channel;
    int noteNo;
    int velocity;

    void *sem;
    bool isGraceNote;
} Note;

#define NoteCreate() calloc(1, sizeof(Note))
#define NoteDestroy  free

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
        int numerator;
        int denominator;
    } time;

    struct {
        SEMMeter *meter;
        SEMTempo *tempo;
        int unitNoteLength;
        int tick;

        struct {
            bool untilBar;
            bool allOctave;
        } accidental;
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
} ABCSEMAnalyzer;

static int VoiceIdComparator(const void *_id1, const void *_id2);
#define inFileFeader(self) (NULL == self->tune)
static void flushPendingNotes(ABCSEMAnalyzer *self, VoiceContext *voice);

static Node *process(void *self, Node *node)
{
    node->accept(node, self);
    return NULL;
}

static void destroy(void *_self)
{
    ABCSEMAnalyzer *self = _self;
    NAMapTraverseValue(self->repeatMap, RepeatContextDestroy);
    NAMapDestroy(self->repeatMap);
    NAMapTraverseValue(self->voiceMap, VoiceContextDestroy);
    NAMapDestroy(self->voiceMap);
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

        self->time.numerator = self->file.meter->numerator;
        self->time.denominator = self->file.meter->denominator;
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
        voice->accidental.untilBar = self->file.accidental.untilBar;
        voice->accidental.allOctave = self->file.accidental.allOctave;
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
        flushPendingNotes(self, voice);

        tick = MAX(tick, voice->tick);
    }

    self->builder->setLength(self->builder, tick);
    self->file.tick = tick + RESOLUTION * 4;
}

static void resetBrokenRhythm(VoiceContext *voice)
{
    voice->brokenRhythm.prev.multiplier = 1;
    voice->brokenRhythm.prev.divider = 1;
    voice->brokenRhythm.next.multiplier = 1;
    voice->brokenRhythm.next.divider = 1;
}

static void visitTune(void *_self, SEMTune *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (!NAArrayIsEmpty(sem->titleList)) {
        char *titleList = NACStringJoinWithStringArray("; ", sem->titleList);
        self->builder->appendTitle(self->builder, self->file.tick, titleList);
        free(titleList);
    }

    if (!NAArrayIsEmpty(sem->copyrightList)) {
        char *copyrightList = NACStringJoinWithStringArray("; ", sem->copyrightList);
        self->builder->appendCopyright(self->builder, self->file.tick, copyrightList);
        free(copyrightList);
    }

    NAMapTraverseValue(self->repeatMap, RepeatContextDestroy);
    NAMapRemoveAll(self->repeatMap);
    NAMapTraverseValue(self->voiceMap, VoiceContextDestroy);
    NAMapRemoveAll(self->voiceMap);
    
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
        resetBrokenRhythm(context);

        NAMapPut(self->voiceMap, voiceIds[i], context);
    }

    setFileContext(self);

    int length = sem->partSequence ? strlen(sem->partSequence) + 1 : 1;
    char *partSequence = alloca(length + 1);
    sprintf(partSequence, "#%s", sem->partSequence);

    for (int i = 0; i < length; ++i) {
        Node *part = NAMapGet(sem->partMap, NACStringFromChar(partSequence[i]));
        if (part) {
            part->accept(part, self);
        }
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
        return;
    }

    self->time.numerator = sem->numerator;
    self->time.denominator = sem->denominator;

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

static void visitUnitNoteLength(void *_self, SEMUnitNoteLength *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (inFileFeader(self)) {
        self->file.unitNoteLength = sem->length;
        return;
    }

    self->voice->unitNoteLength = sem->length;
}

static void visitTempo(void *_self, SEMTempo *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (inFileFeader(self)) {
        self->file.tempo = sem;
        return;
    }

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
            self->builder->appendTempo(self->builder, self->pending.tempo.tick, _sem->tempo);
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

    self->voice->lastBarTick = self->voice->tick;
    self->voice->preIncrementTick = self->voice->tick;

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

static void flushPendingNotes(ABCSEMAnalyzer *self, VoiceContext *voice)
{
    int increment = 0;
    bool resetGraceNoteLength = false;

    NAIterator *iterator = NAArrayGetIterator(voice->pendingNotes);
    while (iterator->hasNext(iterator)) {
        Note *note = iterator->next(iterator);

        int multiplier = note->isGraceNote ? 1 : voice->brokenRhythm.prev.multiplier;
        int divider = note->isGraceNote ? 1 : voice->brokenRhythm.prev.divider;

        if (0 != note->step * multiplier % divider) {
            float result = (float)(note->step * multiplier) / (float)divider;
            appendError(self, note->sem, ABCParseErrorInvalidCaluculatedNoteLength, NACStringFromFloat(result, 2), NACStringFromInteger(RESOLUTION), NULL);
        }
        else {
            
            if (note->isGraceNote) {
                voice->tick += increment;
                increment = 0;
            }

            int step = note->step * multiplier / divider;

            if (note->isGraceNote) {
                voice->graceNoteLength += step;
            }
            else {
                if (0 < voice->graceNoteLength) {
                    resetGraceNoteLength = true;

                    if (voice->graceNoteLength < step) {
                        step -= voice->graceNoteLength;
                    }
                }
            }

            int velocity = voice->accent ? MIN(note->velocity + 20, 127) : note->velocity;

            if (-1 != note->channel) {
                self->builder->appendNote(self->builder, voice->tick, note->channel, note->noteNo, step, velocity);
            }

            increment = MAX(increment, step);
        }

        NoteDestroy(note);
    }

    NAArrayRemoveAll(voice->pendingNotes);

    voice->tick += increment;
    voice->preIncrementTick = voice->tick;

    if (!voice->inChord) {
        resetBrokenRhythm(voice);
    }

    voice->accent = false;
    voice->tie = false;

    if (resetGraceNoteLength) {
        voice->graceNoteLength = 0;
    }
}

static void popTupletStack(VoiceContext *voice)
{
    if (voice->tuplet) {
        if (0 == --voice->tuplet->count) {
            TupletDestroy(voice->tuplet);
            voice->tuplet = NAStackPop(voice->tupletStack);
        }
    }
}

static bool calcNextStep(ABCSEMAnalyzer *self, NoteLength *length, void *sem, int *result)
{
    VoiceContext *voice = self->voice;

    int multiplier = length->multiplier;
    int divider = length->divider;

    if (!voice->inGraceNote) {
        multiplier *= voice->brokenRhythm.next.multiplier;
        divider *= voice->brokenRhythm.next.divider;

        if (voice->tuplet) {
            multiplier *= voice->tuplet->time;
            divider *= voice->tuplet->division;

            if (!voice->inChord) {
                popTupletStack(voice);
            }
        }
    }

    if (0 != voice->unitNoteLength * multiplier % divider) {
        float result = (float)(voice->unitNoteLength * multiplier) / (float)divider;
        appendError(self, sem, ABCParseErrorInvalidCaluculatedNoteLength, NACStringFromFloat(result, 2), NACStringFromInteger(RESOLUTION), NULL);
        return false;
    }

    if (voice->inGraceNote) {
        divider *= 4;
    }

    *result = voice->unitNoteLength * multiplier / divider;
    return true;
}

static bool processTie(ABCSEMAnalyzer *self, SEMNote *sem, int step, int noteNo)
{
#define isNeighbor(b1, b2) (1 > abs((int)b1 - (int)b2) || 5 < abs((int)b1 - (int)b2))

    if (!self->voice->tie) {
        return false;
    }

    bool proccessed = false;

    NAIterator *iterator = NAArrayGetIterator(self->voice->pendingNotes);
    while (iterator->hasNext(iterator)) {
        Note *_note = iterator->next(iterator);
        if (-1 != _note->channel) {
            SEMNote *note = _note->sem;
            if (isNeighbor(note->baseNote, sem->baseNote) && 6 > abs(_note->noteNo - noteNo)) {
                _note->step += step;
                self->voice->tie = false;
                proccessed = true;
                break;
            }
        }
    }

    return proccessed;
#undef isNeighbor
}

static void visitNote(void *_self, SEMNote *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (RepeatStateInitial != self->repeat->state) {
        return;
    }

#if 0
    printf("[REPEAT TEST] baseNote=%s\n", BaseNote2String(sem->baseNote));
#endif

    VoiceContext *voice = self->voice;

    int octave = self->key->octave + voice->octave;
    int transpose = self->key->transpose + voice->transpose;

    int noteOctave = octave + sem->octave;
    Accidental accidental = sem->accidental;

    if (AccidentalNone != accidental) {
        if (voice->accidental.untilBar) {
            voice->accidental.notes[sem->baseNote].accidental = accidental;
            voice->accidental.notes[sem->baseNote].octave = noteOctave;
        }
    }
    else {
        if (AccidentalNone != voice->accidental.notes[sem->baseNote].accidental) {
            if (voice->accidental.allOctave || noteOctave == voice->accidental.notes[sem->baseNote].octave) {
                accidental = voice->accidental.notes[sem->baseNote].accidental;
            }
        }
    }

    int noteNo = NoteTableGetNoteNo(self->key->noteTable, sem->baseNote, accidental, noteOctave);
    noteNo += transpose;

    if (!isValidRange(noteNo, 0, 127)) {
        appendError(self, sem, ABCParseErrorInvalidNoteNumber, NACStringFromInteger(noteNo),
                NACStringFromInteger(octave), NACStringFromInteger(transpose), sem->noteString, NULL);
        return;
    }

    int step;
    if (!calcNextStep(self, &sem->length, sem, &step)) {
        return;
    }

    if (processTie(self, sem, step, noteNo)) {
        return;
    }

    if (!voice->inChord && !voice->inGraceNote) {
        flushPendingNotes(self, voice);
    }

    Note *note = NoteCreate();
    note->step = step;
    note->channel = voice->channel;
    note->noteNo = noteNo;
    note->velocity = voice->velocity;
    note->sem = sem;
    note->isGraceNote = voice->inGraceNote;

    NAArrayAppend(self->voice->pendingNotes, note);

    if (!voice->inChord) {
        voice->preIncrementTick += step;
    }
}

static void visitBrokenRhythm(void *_self, SEMBrokenRhythm *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (RepeatStateInitial != self->repeat->state) {
        return;
    }

    switch (sem->direction) {
    case '<':
        self->voice->brokenRhythm.prev.divider *= 2;

        self->voice->brokenRhythm.next.multiplier *= 3;
        self->voice->brokenRhythm.next.divider *= 2;
        break;
    case '>':
        self->voice->brokenRhythm.prev.multiplier *= 3;
        self->voice->brokenRhythm.prev.divider *= 2;

        self->voice->brokenRhythm.next.divider *= 2;
        break;
    }
}

static void visitRest(void *_self, SEMRest *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (RepeatStateInitial != self->repeat->state) {
        return;
    }

    int step;

    switch (sem->type) {
    case RestUnitNote:
        if (!calcNextStep(self, &sem->length, sem, &step)) {
            return;
        }
        break;
    case RestMeasure:
        step = RESOLUTION * 4 * self->time.numerator / self->time.denominator;
        break;
    }

    flushPendingNotes(self, self->voice);

    Note *note = NoteCreate();
    note->step = step;
    note->channel = -1;
    note->sem = sem;

    NAArrayAppend(self->voice->pendingNotes, note);
}

static void visitRepeat(void *_self, SEMRepeat *sem)
{
    ABCSEMAnalyzer *self = _self;

    switch (sem->type) {
    case RepeatStart:
        switch (self->repeat->state) {
        case RepeatStateInitial:
            self->repeat->state = RepeatStateStart;
            self->repeat->nth = 1;
            break;
        case RepeatStateStart:
        case RepeatStateEnd:
        case RepeatStateNthSearch:
            break;
        }
        break;
    case RepeatEnd:
        switch (self->repeat->state) {
        case RepeatStateInitial:
            if (!NASetContains(self->repeat->passedEndSet, &self->repeat->currentIndex)) {
                self->repeat->state = RepeatStateEnd;
            }
            break;
        case RepeatStateStart:
        case RepeatStateEnd:
        case RepeatStateNthSearch:
            break;
        }
        break;
    case RepeatNth:
        switch (self->repeat->state) {
        case RepeatStateInitial:
            if (!NASetContains(sem->nthSet, &self->repeat->nth)) {
                self->repeat->state = RepeatStateNthSearch;
            }
            break;
        case RepeatStateStart:
        case RepeatStateEnd:
            break;
        case RepeatStateNthSearch:
            if (NASetContains(sem->nthSet, &self->repeat->nth)) {
                self->repeat->state = RepeatStateInitial;
            }
            break;
        }
        break;
    }

    self->voice->lastBarTick = self->voice->preIncrementTick;
}

static void visitBarLine(void *_self, SEMBarLine *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (RepeatStateInitial != self->repeat->state) {
        return;
    }

    self->voice->lastBarTick = self->voice->preIncrementTick;

    for (int i = 0; i < BaseNoteCount; ++i) {
        self->voice->accidental.notes[i].accidental = AccidentalNone;
    }
}

static void visitTie(void *_self, SEMTie *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (RepeatStateInitial != self->repeat->state) {
        return;
    }
    
    if (self->voice->tie) {
        appendError(self, sem, ABCParseErrorIllegalTie, NULL);
    }

    self->voice->tie = true;
}

static void visitGraceNote(void *_self, SEMGraceNote *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (RepeatStateInitial != self->repeat->state) {
        return;
    }

    VoiceContext *voice = self->voice;

    voice->inGraceNote = true;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    voice->inGraceNote = false;
}

static void visitTuplet(void *_self, SEMTuplet *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (RepeatStateInitial != self->repeat->state) {
        return;
    }
    
    Tuplet *tuplet = TupletCreate();
    tuplet->division = sem->division;
    tuplet->time = sem->time;
    tuplet->count = sem->count;

    Tuplet *current = self->voice->tuplet;

    if (current) {
        tuplet->division *= current->division;

        if (0 == --current->count) {
            TupletDestroy(current);
        }
        else {
            NAStackPush(self->voice->tupletStack, current);
        }
    }

    self->voice->tuplet = tuplet;
}

static void visitChord(void *_self, SEMChord *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (RepeatStateInitial != self->repeat->state) {
        return;
    }

    VoiceContext *voice = self->voice;

    voice->inChord = true;

    flushPendingNotes(self, voice);

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    popTupletStack(voice);
    resetBrokenRhythm(voice);

    voice->inChord = false;
}

static void visitOverlay(void *_self, SEMOverlay *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (RepeatStateInitial != self->repeat->state) {
        return;
    }

    if (self->voice->lastBarTick == self->voice->preIncrementTick) {
        appendError(self, sem, ABCParseErrorIllegalOverlay, NULL);
    }
    else {
        flushPendingNotes(self, self->voice);

        self->voice->tick = self->voice->lastBarTick;
    }
}

static void visitMidiVoice(void *_self, SEMMidiVoice *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (!inFileFeader(self) && RepeatStateInitial == self->repeat->state) {
        flushPendingNotes(self, self->voice);
    }

    int channel = self->voice->channel;
    
    if (-1 != sem->instrument) {
        int msb = 0x7F & ((sem->bank - 1) >> 7);
        int lsb = 0x7F & ((sem->bank - 1) >> 0);
        int programNo = sem->instrument - 1;

        self->builder->appendVoice(self->builder, self->voice->tick, channel, msb, lsb, programNo);
    }

    self->builder->appendVolume(self->builder, self->voice->tick, channel, sem->mute ? 0 : 100);
}

static void visitPropagateAccidental(void *_self, SEMPropagateAccidental *sem)
{
    ABCSEMAnalyzer *self = _self;

    if (!inFileFeader(self) && RepeatStateInitial == self->repeat->state) {
        flushPendingNotes(self, self->voice);
    }

    if (inFileFeader(self)) {
        self->file.accidental.untilBar = sem->untilBar;
        self->file.accidental.allOctave = sem->allOctave;
    }
    else {
        self->voice->accidental.untilBar = sem->untilBar;
        self->voice->accidental.allOctave = sem->allOctave;
    }
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

    self->file.unitNoteLength = RESOLUTION / 2;
    self->file.accidental.untilBar = true;
    self->file.accidental.allOctave = true;

    self->time.numerator = 4;
    self->time.denominator = 4;

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
    VoiceContext *self = calloc(1, sizeof(VoiceContext));
    self->pendingNotes = NAArrayCreate(4, NADescriptionAddress);
    self->tupletStack = NAStackCreate(4);
    return self;
}

static void VoiceContextDestroy(VoiceContext *self)
{
    NAArrayTraverse(self->pendingNotes, NoteDestroy);
    NAArrayDestroy(self->pendingNotes);

    Tuplet *tuplet;
    while ((tuplet = NAStackPop(self->tupletStack))) {
        TupletDestroy(tuplet);
    }
    NAStackDestroy(self->tupletStack);

    free(self);
}
