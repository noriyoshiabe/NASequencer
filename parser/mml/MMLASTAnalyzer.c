#include "MMLASTAnalyzer.h"
#include "MMLParser.h"
#include "MMLAST.h"
#include "MMLSEM.h"
#include "NAStack.h"
#include "NACString.h"
#include "NALog.h"

#include <stdlib.h>
#include <ctype.h>

#define node(type, ast) MMLSEM##type##Create(&ast->node.location)
#define append(list, sem) NAArrayAppend(list, sem)
#define appendError(self, ast, ...) self->context->appendError(self->context, &ast->node.location, __VA_ARGS__)

#define isValidRange(v, from, to) (from <= v && v <= to)

#define isGlobal(state) (GLOBAL == state->name)

static const char *GLOBAL = "global";
static const char *TUPLET = "tuplet";
static const char *REPEAT = "repeat";
static const char *CHORD = "chord";

typedef struct _State {
    NAArray *list;
    const char *name;
} State;

typedef struct _MMLASTAnalyzer {
    ASTVisitor visitor;
    Analyzer analyzer;
    ParseContext *context;
    State *state;
    NAStack *stateStack;
} MMLASTAnalyzer;

static BaseNote KeyChar2BaseNote(char c);
static State *StateCreate(NAArray *list, const char *name);
static void StateDestroy(State *self);

static Node *process(void *_self, Node *node)
{
    MMLASTAnalyzer *self = _self;

    SEMList *list = MMLSEMListCreate(NULL);
    self->state = StateCreate(list->node.children, GLOBAL);

    node->accept(node, self);

    return (Node *)list;
}

static void destroy(void *_self)
{
    MMLASTAnalyzer *self = _self;
    NAStackDestroy(self->stateStack);
    StateDestroy(self->state);
    free(self);
}

static void visitRoot(void *_self, ASTRoot *ast)
{
    MMLASTAnalyzer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitTimebase(void *_self, ASTTimebase *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isGlobal(self->state)) {
        appendError(self, ast, MMLParseErrorIllegalStateWithTimebase, self->state->name, NULL);
        return;
    }

    if (!isValidRange(ast->timebase, 48, 9600)) {
        appendError(self, ast, MMLParseErrorInvalidTimebase, NACStringFromInteger(ast->timebase), NULL);
        return;
    }
    
    SEMTimebase *sem = node(Timebase, ast);
    sem->timebase = ast->timebase;
    append(self->state->list, sem);
}

static void visitTitle(void *_self, ASTTitle *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isGlobal(self->state)) {
        appendError(self, ast, MMLParseErrorIllegalStateWithTitle, self->state->name, NULL);
        return;
    }

    SEMTitle *sem = node(Title, ast);
    sem->title = strdup(ast->title);
    append(self->state->list, sem);
}

static void visitCopyright(void *_self, ASTCopyright *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isGlobal(self->state)) {
        appendError(self, ast, MMLParseErrorIllegalStateWithCopyright, self->state->name, NULL);
        return;
    }

    SEMCopyright *sem = node(Copyright, ast);
    sem->text = strdup(ast->text);
    append(self->state->list, sem);
}

static void visitMarker(void *_self, ASTMarker *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isGlobal(self->state)) {
        appendError(self, ast, MMLParseErrorIllegalStateWithMarker, self->state->name, NULL);
        return;
    }

    SEMMarker *sem = node(Marker, ast);
    sem->text = strdup(ast->text);
    append(self->state->list, sem);
}

static void visitVelocityReverse(void *_self, ASTVelocityReverse *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isGlobal(self->state)) {
        appendError(self, ast, MMLParseErrorIllegalStateWithVelocityReverse, self->state->name, NULL);
        return;
    }

    SEMVelocityReverse *sem = node(VelocityReverse, ast);
    append(self->state->list, sem);
}

static void visitOctaveReverse(void *_self, ASTOctaveReverse *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isGlobal(self->state)) {
        appendError(self, ast, MMLParseErrorIllegalStateWithOctaveReverse, self->state->name, NULL);
        return;
    }

    SEMOctaveReverse *sem = node(OctaveReverse, ast);
    append(self->state->list, sem);
}

static void visitChannel(void *_self, ASTChannel *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isValidRange(ast->number, 1, 16)) {
        appendError(self, ast, MMLParseErrorInvalidChannel, NACStringFromInteger(ast->number), NULL);
        return;
    }

    SEMChannel *sem = node(Channel, ast);
    sem->number = ast->number;
    append(self->state->list, sem);
}

static void visitSynth(void *_self, ASTSynth *ast)
{
    MMLASTAnalyzer *self = _self;

    SEMSynth *sem = node(Synth, ast);
    sem->name = strdup(ast->name);
    append(self->state->list, sem);
}

static void visitBankSelect(void *_self, ASTBankSelect *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isValidRange(ast->msb, 0, 127) || !isValidRange(ast->lsb, 0, 127)) {
        appendError(self, ast, MMLParseErrorInvalidBankSelect, NACStringFromInteger(ast->msb), NACStringFromInteger(ast->lsb), NULL);
        return;
    }

    SEMBankSelect *sem = node(BankSelect, ast);
    sem->msb = ast->msb;
    sem->lsb = ast->lsb;
    append(self->state->list, sem);
}

static void visitProgramChange(void *_self, ASTProgramChange *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isValidRange(ast->programNo, 0, 127)) {
        appendError(self, ast, MMLParseErrorInvalidProgramChange, NACStringFromInteger(ast->programNo), NULL);
        return;
    }

    SEMProgramChange *sem = node(ProgramChange, ast);
    sem->programNo = ast->programNo;
    append(self->state->list, sem);
}

static void visitVolume(void *_self, ASTVolume *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isValidRange(ast->value, 0, 127)) {
        appendError(self, ast, MMLParseErrorInvalidVolume, NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMVolume *sem = node(Volume, ast);
    sem->value = ast->value;
    append(self->state->list, sem);
}

static void visitChorus(void *_self, ASTChorus *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isValidRange(ast->value, 0, 127)) {
        appendError(self, ast, MMLParseErrorInvalidChorus, NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMChorus *sem = node(Chorus, ast);
    sem->value = ast->value;
    append(self->state->list, sem);
}

static void visitReverb(void *_self, ASTReverb *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isValidRange(ast->value, 0, 127)) {
        appendError(self, ast, MMLParseErrorInvalidReverb, NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMReverb *sem = node(Reverb, ast);
    sem->value = ast->value;
    append(self->state->list, sem);
}

static void visitExpression(void *_self, ASTExpression *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isValidRange(ast->value, 0, 127)) {
        appendError(self, ast, MMLParseErrorInvalidExpression, NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMExpression *sem = node(Expression, ast);
    sem->value = ast->value;
    append(self->state->list, sem);
}

static void visitPan(void *_self, ASTPan *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isValidRange(ast->value, 0, 127)) {
        appendError(self, ast, MMLParseErrorInvalidPan, NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMPan *sem = node(Pan, ast);
    sem->value = ast->value;
    append(self->state->list, sem);
}

static void visitDetune(void *self, ASTDetune *ast)
{
    __Trace__
}

static void visitTempo(void *self, ASTTempo *ast)
{
    __Trace__
}

static void visitNote(void *self, ASTNote *ast)
{
    __Trace__
}

static void visitRest(void *self, ASTRest *ast)
{
    __Trace__
}

static void visitOctave(void *self, ASTOctave *ast)
{
    __Trace__
}

static void visitTransepose(void *self, ASTTransepose *ast)
{
    __Trace__
}

static void visitTie(void *self, ASTTie *ast)
{
    __Trace__
}

static void visitLength(void *self, ASTLength *ast)
{
    __Trace__
}

static void visitGatetime(void *self, ASTGatetime *ast)
{
    __Trace__
}

static void visitVelocity(void *self, ASTVelocity *ast)
{
    __Trace__
}

static void visitTuplet(void *_self, ASTTuplet *ast)
{
    MMLASTAnalyzer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitTrackChange(void *self, ASTTrackChange *ast)
{
    __Trace__
}

static void visitRepeat(void *_self, ASTRepeat *ast)
{
    MMLASTAnalyzer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitRepeatBreak(void *self, ASTRepeatBreak *ast)
{
    __Trace__
}

static void visitChord(void *_self, ASTChord *ast)
{
    MMLASTAnalyzer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

Analyzer *MMLASTAnalyzerCreate(ParseContext *context)
{
    MMLASTAnalyzer *self = calloc(1, sizeof(MMLASTAnalyzer));

    self->visitor.visitRoot = visitRoot;
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

static State *StateCreate(NAArray *list, const char *name)
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
