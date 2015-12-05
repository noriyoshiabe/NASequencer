#include "NAMidiASTAnalyzer.h"
#include "NAMidiParser.h"
#include "NAMidiAST.h"
#include "NAMidiSEM.h"
#include "NAStack.h"
#include "NASet.h"
#include "NACString.h"
#include "NAUtil.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define node(type, ast) NAMidiSEM##type##Create(&ast->node.location)
#define append(list, sem) NAArrayAppend(list->node.children, sem)
#define appendError(self, ast, ...) self->context->appendError(self->context, &ast->node.location, __VA_ARGS__)

#define isValidRange(v, from, to) (from <= v && v <= to)
#define isPowerOf2(x) ((x != 0) && ((x & (x - 1)) == 0))

static const char *GLOBAL = "global";
static const char *PATTERN = "pattern";
static const char *CONTEXT = "context";

typedef struct _State {
    SEMList *list;
    const char *name;
} State;

typedef struct _NAMidiASTAnalyzer {
    ASTVisitor visitor;
    Analyzer analyzer;
    ParseContext *context;
    State *state;
    NAStack *stateStack;
} NAMidiASTAnalyzer;

static BaseNote KeyChar2BaseNote(char c);
static State *StateCreate(SEMList *list, const char *name);
static void StateDestroy(State *self);

static Node *process(void *_self, Node *node)
{
    NAMidiASTAnalyzer *self = _self;

    SEMList *list = NAMidiSEMListCreate(NULL);
    self->state = StateCreate(list, GLOBAL);

    node->accept(node, self);

    return (Node *)list;
}

static void destroy(void *_self)
{
    NAMidiASTAnalyzer *self = _self;
    NAStackDestroy(self->stateStack);
    StateDestroy(self->state);
    free(self);
}

static void visitRoot(void *_self, ASTRoot *ast)
{
    NAMidiASTAnalyzer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitResolution(void *_self, ASTResolution *ast)
{
    NAMidiASTAnalyzer *self = _self;

    if (GLOBAL != self->state->name) {
        appendError(self, ast, NAMidiParseErrorIllegalStateWithResolution, self->state->name, NULL);
        return;
    }

    if (!isValidRange(ast->resolution, 1, 9600)) {
        appendError(self, ast, NAMidiParseErrorInvalidResolution, NACStringFromInteger(ast->resolution), NULL);
        return;
    }
    
    SEMResolution *sem = node(Resolution, ast);
    sem->resolution = ast->resolution;
    append(self->state->list, sem);
}

static void visitTitle(void *_self, ASTTitle *ast)
{
    NAMidiASTAnalyzer *self = _self;

    if (GLOBAL != self->state->name) {
        appendError(self, ast, NAMidiParseErrorIllegalStateWithTitle, self->state->name, NULL);
        return;
    }

    SEMTitle *sem = node(Title, ast);
    sem->title = strdup(ast->title);
    append(self->state->list, sem);
}

static void visitTempo(void *_self, ASTTempo *ast)
{
    NAMidiASTAnalyzer *self = _self;

    if (!isValidRange(ast->tempo, 30.0, 300.0)) {
        appendError(self, ast, NAMidiParseErrorInvalidTempo, NACStringFromFloat(ast->tempo, 2), NULL);
        return;
    }

    SEMTempo *sem = node(Tempo, ast);
    sem->tempo = ast->tempo;
    append(self->state->list, sem);
}

static void visitTime(void *_self, ASTTime *ast)
{
    NAMidiASTAnalyzer *self = _self;

    if (1 > ast->numerator || 1 > ast->denominator || !isPowerOf2(ast->denominator)) {
        appendError(self, ast, NAMidiParseErrorInvalidTimeSign, NACStringFromInteger(ast->numerator), NACStringFromInteger(ast->denominator), NULL);
        return;
    }

    SEMTime *sem = node(Time, ast);
    sem->numerator = ast->numerator;
    sem->denominator = ast->denominator;
    append(self->state->list, sem);
}

static void visitKey(void *_self, ASTKey *ast)
{
    NAMidiASTAnalyzer *self = _self;

    BaseNote baseNote = KeyChar2BaseNote(ast->keyString[0]);
    bool sharp = NULL != strchr(&ast->keyString[1], '#');
    bool flat = NULL != strchr(&ast->keyString[1], 'b');
    Mode mode = NULL != strstr(&ast->keyString[1], "min") ? ModeMinor : ModeMajor;

    NoteTable *noteTable = NoteTableCreate(baseNote, sharp, flat, mode);
    if (NoteTableHasUnusualKeySign(noteTable)) {
        appendError(self, ast, NAMidiParseErrorInvalidKeySign, ast->keyString, NULL);
        NoteTableRelease(noteTable);
        return;
    }

    SEMKey *sem = node(Key, ast);
    sem->noteTable = noteTable;
    append(self->state->list, sem);
}

static void visitMarker(void *_self, ASTMarker *ast)
{
    NAMidiASTAnalyzer *self = _self;

    SEMMarker *sem = node(Marker, ast);
    sem->text = strdup(ast->text);
    append(self->state->list, sem);
}

static void visitChannel(void *_self, ASTChannel *ast)
{
    NAMidiASTAnalyzer *self = _self;

    if (!isValidRange(ast->number, 1, 16)) {
        appendError(self, ast, NAMidiParseErrorInvalidChannel, NACStringFromInteger(ast->number), NULL);
        return;
    }

    SEMChannel *sem = node(Channel, ast);
    sem->number = ast->number;
    append(self->state->list, sem);
}

static void visitVoice(void *_self, ASTVoice *ast)
{
    NAMidiASTAnalyzer *self = _self;

    if (!isValidRange(ast->msb, 0, 127) || !isValidRange(ast->lsb, 0, 127) || !isValidRange(ast->programNo, 0, 127)) {
        appendError(self, ast, NAMidiParseErrorInvalidVoice,
                NACStringFromInteger(ast->msb), NACStringFromInteger(ast->lsb), NACStringFromInteger(ast->programNo), NULL);
        return;
    }

    SEMVoice *sem = node(Voice, ast);
    sem->msb = ast->msb;
    sem->lsb = ast->lsb;
    sem->programNo = ast->programNo;
    append(self->state->list, sem);
}

static void visitSynth(void *_self, ASTSynth *ast)
{
    NAMidiASTAnalyzer *self = _self;

    SEMSynth *sem = node(Synth, ast);
    sem->name = strdup(ast->name);
    append(self->state->list, sem);
}

static void visitVolume(void *_self, ASTVolume *ast)
{
    NAMidiASTAnalyzer *self = _self;

    if (!isValidRange(ast->value, 0, 127)) {
        appendError(self, ast, NAMidiParseErrorInvalidVolume, NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMVolume *sem = node(Volume, ast);
    sem->value = ast->value;
    append(self->state->list, sem);
}

static void visitPan(void *_self, ASTPan *ast)
{
    NAMidiASTAnalyzer *self = _self;

    if (!isValidRange(ast->value, -64, 64)) {
        appendError(self, ast, NAMidiParseErrorInvalidPan, NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMPan *sem = node(Pan, ast);
    sem->value = ast->value;
    append(self->state->list, sem);
}

static void visitChorus(void *_self, ASTChorus *ast)
{
    NAMidiASTAnalyzer *self = _self;

    if (!isValidRange(ast->value, 0, 127)) {
        appendError(self, ast, NAMidiParseErrorInvalidChorus, NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMChorus *sem = node(Chorus, ast);
    sem->value = ast->value;
    append(self->state->list, sem);
}

static void visitReverb(void *_self, ASTReverb *ast)
{
    NAMidiASTAnalyzer *self = _self;

    if (!isValidRange(ast->value, 0, 127)) {
        appendError(self, ast, NAMidiParseErrorInvalidReverb, NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMReverb *sem = node(Reverb, ast);
    sem->value = ast->value;
    append(self->state->list, sem);
}

static void visitTranspose(void *_self, ASTTranspose *ast)
{
    NAMidiASTAnalyzer *self = _self;

    if (!isValidRange(ast->value, -64, 64)) {
        appendError(self, ast, NAMidiParseErrorInvalidTranspose, NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMTranspose *sem = node(Transpose, ast);
    sem->value = ast->value;
    append(self->state->list, sem);
}

static void visitRest(void *_self, ASTRest *ast)
{
    NAMidiASTAnalyzer *self = _self;

    if (!isValidRange(ast->step, 0, 65535)) {
        appendError(self, ast, NAMidiParseErrorInvalidStep, NACStringFromInteger(ast->step), NULL);
        return;
    }

    SEMRest *sem = node(Rest, ast);
    sem->step = ast->step;
    append(self->state->list, sem);
}

static void visitNote(void *_self, ASTNote *ast)
{
    NAMidiASTAnalyzer *self = _self;

    int step = -1;
    int gatetime = -1;
    int velocity = -1;

    int count = NAArrayCount(ast->node.children);
    for (int i = 0; i < count; ++i) {
        ASTNoteParam *np = NAArrayGetValueAt(ast->node.children, i);
        switch (i) {
        case 0:
            step = np->value;
            if (!isValidRange(step, -1, 65535)) {
                appendError(self, ast, NAMidiParseErrorInvalidStep, NACStringFromInteger(step), NULL);
                return;
            }
            break;
        case 1:
            gatetime = np->value;
            if (!isValidRange(gatetime, -1, 65535)) {
                appendError(self, ast, NAMidiParseErrorInvalidGatetime, NACStringFromInteger(gatetime), NULL);
                return;
            }
            break;
        case 2:
            velocity = np->value;
            if (!isValidRange(velocity, -1, 127)) {
                appendError(self, ast, NAMidiParseErrorInvalidVelocity, NACStringFromInteger(velocity), NULL);
                return;
            }
            break;
        default:
            appendError(self, ast, NAMidiParseErrorTooManyNoteParams, NACStringFromInteger(np->value), NULL);
            return;
        }
    }

    char *pc = ast->noteString;

    BaseNote baseNote = KeyChar2BaseNote(*pc);
    Accidental accidental = AccidentalNone;

    int octave = SEMNOTE_OCTAVE_NONE;
    char *c;
    while (*(c = ++pc)) {
        switch (*c) {
        case '#':
            accidental = AccidentalSharp == accidental ? AccidentalDoubleSharp : AccidentalSharp;
            break;
        case 'b':
            accidental = AccidentalFlat == accidental ? AccidentalDoubleFlat : AccidentalFlat;
            break;
        case 'n':
            accidental = AccidentalNatural;
            break;
        default:
            octave = atoi(c);
            break;
        }

        if (SEMNOTE_OCTAVE_NONE != octave) {
            break;
        }
    }

    if (SEMNOTE_OCTAVE_NONE != octave && !isValidRange(octave, -2, 8)) {
        appendError(self, ast, NAMidiParseErrorInvalidOctave, NACStringFromInteger(octave), ast->noteString, NULL);
        return;
    }

    SEMNote *sem = node(Note, ast);
    sem->step = step;
    sem->baseNote = baseNote;
    sem->accidental = accidental;
    sem->octave = octave;
    sem->gatetime = gatetime;
    sem->velocity = velocity;
    sem->noteString = strdup(ast->noteString);
    append(self->state->list, sem);
}

static void visitInclude(void *_self, ASTInclude *ast)
{
    NAMidiASTAnalyzer *self = _self;

    if (GLOBAL != self->state->name) {
        appendError(self, ast, NAMidiParseErrorIllegalStateWithInclude, self->state->name, NULL);
        return;
    }

    if (ast->root) {
        ast->root->accept(ast->root, self);
    }
}

static void visitPattern(void *_self, ASTPattern *ast)
{
    NAMidiASTAnalyzer *self = _self;

    SEMPattern *sem = node(Pattern, ast);
    sem->identifier = strdup(ast->identifier);

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        ASTIdentifier *node = iterator->next(iterator);
        NAArrayAppend(sem->ctxIdList, strdup(node->idString));
    }

    if (NAArrayIsEmpty(sem->ctxIdList)) {
        NAArrayAppend(sem->ctxIdList, strdup("default"));
    }

    append(self->state->list, sem);
}

static void visitDefine(void *_self, ASTDefine *ast)
{
    NAMidiASTAnalyzer *self = _self;

    if (CONTEXT == self->state->name) {
        appendError(self, ast, NAMidiParseErrorIllegalStateWithDefine, self->state->name, NULL);
        return;
    }

    if (NAMapContainsKey(self->state->list->patternMap, ast->identifier)) {
        Node *original = NAMapGet(self->state->list->patternMap, ast->identifier);
        appendError(self, ast, NAMidiParseErrorDuplicatePatternIdentifier,
                ast->identifier, original->location.filepath, NACStringFromInteger(original->location.line), NULL);
        return;
    }

    SEMList *list = node(List, ast);
    list->identifier = strdup(ast->identifier);
    NAMapPut(self->state->list->patternMap, list->identifier, list);

    NAStackPush(self->stateStack, self->state);
    self->state = StateCreate(list, PATTERN);

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    State *local = self->state;
    self->state = NAStackPop(self->stateStack);
    StateDestroy(local);
}

static void visitContext(void *_self, ASTContext *ast)
{
    NAMidiASTAnalyzer *self = _self;

    if (GLOBAL == self->state->name) {
        appendError(self, ast, NAMidiParseErrorIllegalStateWithContext, self->state->name, NULL);
        return;
    }

    SEMContext *sem = node(Context, ast);

    NAIterator *iterator;

    iterator = NAArrayGetIterator(ast->ctxIdList);
    while (iterator->hasNext(iterator)) {
        ASTIdentifier *node = iterator->next(iterator);
        NAArrayAppend(sem->ctxIdList, strdup(node->idString));
    }

    sem->list = node(List, ast);
    NAStackPush(self->stateStack, self->state);
    self->state = StateCreate(sem->list, CONTEXT);

    iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    State *local = self->state;
    self->state = NAStackPop(self->stateStack);
    StateDestroy(local);

    append(self->state->list, sem);
}

static void visitIdentifier(void *_self, ASTIdentifier *ast)
{
    NAMidiASTAnalyzer *self = _self;
}

static void visitNoteParam(void *_self, ASTNoteParam *ast)
{
    NAMidiASTAnalyzer *self = _self;
}

Analyzer *NAMidiASTAnalyzerCreate(ParseContext *context)
{
    NAMidiASTAnalyzer *self = calloc(1, sizeof(NAMidiASTAnalyzer));

    self->visitor.visitRoot = visitRoot;
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
    self->visitor.visitInclude = visitInclude;
    self->visitor.visitPattern = visitPattern;
    self->visitor.visitDefine = visitDefine;
    self->visitor.visitContext = visitContext;
    self->visitor.visitIdentifier = visitIdentifier;
    self->visitor.visitNoteParam = visitNoteParam;

    self->analyzer.process = process;
    self->analyzer.destroy = destroy;
    self->analyzer.self = self;

    self->context = context;
    self->stateStack = NAStackCreate(4);

    return &self->analyzer;
}

static BaseNote KeyChar2BaseNote(char c)
{
    const BaseNote baseNoteTable[] = {
        BaseNote_A, BaseNote_B, BaseNote_C,
        BaseNote_D, BaseNote_E, BaseNote_F, BaseNote_G
    };

    return baseNoteTable[tolower(c) - 97];
}

static State *StateCreate(SEMList *list, const char *name)
{
    State *self = calloc(1, sizeof(State));
    self->list = list;
    self->name = name;
    return self;
}

static void StateDestroy(State *self)
{
    free(self);
}
