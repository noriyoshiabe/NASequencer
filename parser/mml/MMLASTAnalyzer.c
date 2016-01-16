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
#define append(state, sem) NAArrayAppend(state->node->children, sem)
#define appendError(self, ast, ...) self->context->appendError(self->context, &ast->node.location, __VA_ARGS__)

#define isValidRange(v, from, to) (from <= v && v <= to)

#define isGlobal(state) (GLOBAL == state->name)
#define isGlobalOrRepeat(state) (GLOBAL == state->name || REPEAT == state->name)
#define isRepeat(state) (REPEAT == state->name)
#define isChord(state) (CHORD == state->name)

#define inTuplet(self) (self->inTuplet)

static const char *GLOBAL = "global";
static const char *TUPLET = "tuplet";
static const char *REPEAT = "repeat";
static const char *CHORD = "chord";

typedef struct _State {
    Node *node;
    const char *name;
} State;

typedef struct _MMLASTAnalyzer {
    ASTVisitor visitor;
    Analyzer analyzer;
    ParseContext *context;
    State *state;
    NAStack *stateStack;
    bool inTuplet;
} MMLASTAnalyzer;

static BaseNote KeyChar2BaseNote(char c);
static void parseNoteLength(char *string, NoteLength *noteLength);
static State *StateCreate(void *node, const char *name);
static void StateDestroy(State *self);

static Node *process(void *_self, Node *node)
{
    MMLASTAnalyzer *self = _self;

    SEMList *list = MMLSEMListCreate(NULL);
    self->state = StateCreate(list, GLOBAL);

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
    append(self->state, sem);
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
    append(self->state, sem);
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
    append(self->state, sem);
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
    append(self->state, sem);
}

static void visitVelocityReverse(void *_self, ASTVelocityReverse *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isGlobal(self->state)) {
        appendError(self, ast, MMLParseErrorIllegalStateWithVelocityReverse, self->state->name, NULL);
        return;
    }

    SEMVelocityReverse *sem = node(VelocityReverse, ast);
    append(self->state, sem);
}

static void visitOctaveReverse(void *_self, ASTOctaveReverse *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isGlobal(self->state)) {
        appendError(self, ast, MMLParseErrorIllegalStateWithOctaveReverse, self->state->name, NULL);
        return;
    }

    SEMOctaveReverse *sem = node(OctaveReverse, ast);
    append(self->state, sem);
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
    append(self->state, sem);
}

static void visitSynth(void *_self, ASTSynth *ast)
{
    MMLASTAnalyzer *self = _self;

    SEMSynth *sem = node(Synth, ast);
    sem->name = strdup(ast->name);
    append(self->state, sem);
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
    append(self->state, sem);
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
    append(self->state, sem);
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
    append(self->state, sem);
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
    append(self->state, sem);
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
    append(self->state, sem);
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
    append(self->state, sem);
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
    append(self->state, sem);
}

static void visitDetune(void *_self, ASTDetune *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isValidRange(ast->value, -2400, 2400)) {
        appendError(self, ast, MMLParseErrorInvalidDetune, NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMDetune *sem = node(Detune, ast);
    sem->value = ast->value;
    append(self->state, sem);
}

static void visitTempo(void *_self, ASTTempo *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isValidRange(ast->tempo, 30.0, 300.0)) {
        appendError(self, ast, MMLParseErrorInvalidTempo, NACStringFromFloat(ast->tempo, 2), NULL);
        return;
    }

    SEMTempo *sem = node(Tempo, ast);
    sem->tempo = ast->tempo;
    append(self->state, sem);
}

static void visitNote(void *_self, ASTNote *ast)
{
    MMLASTAnalyzer *self = _self;

    BaseNote baseNote = BaseNote_C;
    Accidental accidental = AccidentalNone;
    NoteLength noteLength = {-1, 0, -1};

    char *pc = ast->noteString;
    while (*pc) {
        switch (*pc) {
        case '+':
        case '#':
            accidental = AccidentalSharp == accidental ? AccidentalDoubleSharp : AccidentalSharp;
            break;
        case '-':
            accidental = AccidentalFlat == accidental ? AccidentalDoubleFlat : AccidentalFlat;
            break;
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
            baseNote = KeyChar2BaseNote(*pc);
            break;
        default:
            if (inTuplet(self)) {
                appendError(self, ast, MMLParseErrorIllegalStateWithNoteLength, self->state->name, NACStringFromBoolean(inTuplet(self)), NULL);
                return;
            }

            parseNoteLength(pc, &noteLength);
            if (-1 != noteLength.length && 0 != 384 % noteLength.length) {
                appendError(self, ast, MMLParseErrorInvalidLength, NACStringFromInteger(noteLength.length), NULL);
                return;
            }

            goto LOOP_END;
        }

        ++pc;
    }

LOOP_END:
    ;

    SEMNote *sem = node(Note, ast);
    sem->baseNote = baseNote;
    sem->accidental = accidental;
    sem->length = noteLength;
    sem->noteString = strdup(ast->noteString);
    append(self->state, sem);

    if (inTuplet(self) && !isChord(self->state)) {
        SEMTuplet *tuplet = (SEMTuplet *)self->state->node;
        ++tuplet->division;
    }
}

static void visitRest(void *_self, ASTRest *ast)
{
    MMLASTAnalyzer *self = _self;

    if (inTuplet(self) && isChord(self->state)) {
        appendError(self, ast, MMLParseErrorIllegalStateWithRest, self->state->name, NACStringFromBoolean(inTuplet(self)), NULL);
        return;
    }

    if (inTuplet(self) && '\0' != ast->restString[1] ) {
        appendError(self, ast, MMLParseErrorIllegalStateWithNoteLength, self->state->name, NACStringFromBoolean(inTuplet(self)), NULL);
        return;
    }

    NoteLength noteLength = {-1, 0, -1};
    parseNoteLength(ast->restString + 1, &noteLength);
    if (-1 != noteLength.length && 0 != 384 % noteLength.length) {
        appendError(self, ast, MMLParseErrorInvalidLength, NACStringFromInteger(noteLength.length), NULL);
        return;
    }

    SEMRest *sem = node(Rest, ast);
    sem->length = noteLength;
    append(self->state, sem);

    if (inTuplet(self) && !isChord(self->state)) {
        SEMTuplet *tuplet = (SEMTuplet *)self->state->node;
        ++tuplet->division;
    }
}

static void visitOctave(void *_self, ASTOctave *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isValidRange(ast->value, -2, 8)) {
        appendError(self, ast, MMLParseErrorInvalidOctave, NACStringFromFloat(ast->value, 2), NULL);
        return;
    }

    SEMOctave *sem = node(Octave, ast);
    sem->direction = ast->direction;
    sem->value = ast->value;
    append(self->state, sem);
}

static void visitTransepose(void *_self, ASTTransepose *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isValidRange(ast->value, -64, 64)) {
        appendError(self, ast, MMLParseErrorInvalidTranspose, NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMTranspose *sem = node(Transpose, ast);
    sem->value = ast->value;
    append(self->state, sem);
}

static void visitTie(void *_self, ASTTie *ast)
{
    MMLASTAnalyzer *self = _self;

    if (isChord(self->state)) {
        appendError(self, ast, MMLParseErrorIllegalStateWithTie, self->state->name, NULL);
        return;
    }

    SEMTie *sem = node(Tie, ast);
    append(self->state, sem);
}

static void visitLength(void *_self, ASTLength *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isGlobalOrRepeat(self->state)) {
        appendError(self, ast, MMLParseErrorIllegalStateWithLength, self->state->name, NULL);
        return;
    }

    if (0 != 384 % ast->length) {
        appendError(self, ast, MMLParseErrorInvalidLength, NACStringFromInteger(ast->length), NULL);
        return;
    }

    SEMLength *sem = node(Length, ast);
    sem->length = ast->length;
    append(self->state, sem);
}

static void visitGatetime(void *_self, ASTGatetime *ast)
{
    MMLASTAnalyzer *self = _self;

    if ((!ast->absolute && !isValidRange(ast->value, 0, 16))
            || (ast->absolute && !isValidRange(ast->value, 0, 192))) {
        appendError(self, ast, MMLParseErrorInvalidGatetime, NACStringFromBoolean(ast->absolute), NACStringFromInteger(ast->value), NULL);
        return;
    }

    SEMGatetime *sem = node(Gatetime, ast);
    sem->absolute = ast->absolute;
    sem->value = ast->value;
    append(self->state, sem);
}

static void visitVelocity(void *_self, ASTVelocity *ast)
{
    MMLASTAnalyzer *self = _self;

    if (ast->direction || ast->absolute) {
        if (!isValidRange(ast->value, 0, 127)) {
            appendError(self, ast, MMLParseErrorInvalidVelocity, NACStringFromBoolean(ast->absolute), NACStringFromInteger(ast->value), NULL);
            return;
        }
    }
    else {
        if (!isValidRange(ast->value, 0, 15)) {
            appendError(self, ast, MMLParseErrorInvalidVelocity, NACStringFromBoolean(ast->absolute), NACStringFromInteger(ast->value), NULL);
            return;
        }
    }

    SEMVelocity *sem = node(Velocity, ast);
    sem->direction = ast->direction;
    sem->absolute = ast->absolute;
    sem->value = ast->value;
    append(self->state, sem);
}

static void visitTuplet(void *_self, ASTTuplet *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isGlobalOrRepeat(self->state)) {
        appendError(self, ast, MMLParseErrorIllegalStateWithTuplet, self->state->name, NULL);
        return;
    }

    NoteLength noteLength = {-1, 0, -1};
    parseNoteLength(ast->lengthString, &noteLength);
    if (-1 != noteLength.length && 0 != 384 % noteLength.length) {
        appendError(self, ast, MMLParseErrorInvalidLength, NACStringFromInteger(noteLength.length), NULL);
        return;
    }

    SEMTuplet *sem = node(Tuplet, ast);
    sem->length = noteLength;
    append(self->state, sem);

    NAStackPush(self->stateStack, self->state);
    self->state = StateCreate(sem, TUPLET);
    self->inTuplet = true;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    State *local = self->state;
    self->state = NAStackPop(self->stateStack);
    StateDestroy(local);
    self->inTuplet = false;
}

static void visitTrackChange(void *_self, ASTTrackChange *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isGlobalOrRepeat(self->state)) {
        appendError(self, ast, MMLParseErrorIllegalStateWithTrackChange, self->state->name, NULL);
        return;
    }

    SEMTrackChange *sem = node(TrackChange, ast);
    append(self->state, sem);
}

static void visitRepeat(void *_self, ASTRepeat *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isGlobalOrRepeat(self->state)) {
        appendError(self, ast, MMLParseErrorIllegalStateWithRepeat, self->state->name, NULL);
        return;
    }

    SEMRepeat *sem = node(Repeat, ast);
    sem->times = ast->times;
    append(self->state, sem);

    NAStackPush(self->stateStack, self->state);
    self->state = StateCreate(sem, REPEAT);

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    State *local = self->state;
    self->state = NAStackPop(self->stateStack);
    StateDestroy(local);
}

static void visitRepeatBreak(void *_self, ASTRepeatBreak *ast)
{
    MMLASTAnalyzer *self = _self;

    if (!isRepeat(self->state)) {
        appendError(self, ast, MMLParseErrorIllegalStateWithRepeatBreak, self->state->name, NULL);
        return;
    }

    SEMRepeatBreak *sem = node(RepeatBreak, ast);
    append(self->state, sem);
}

static void visitChord(void *_self, ASTChord *ast)
{
    MMLASTAnalyzer *self = _self;

    if (isChord(self->state)) {
        appendError(self, ast, MMLParseErrorIllegalStateWithChord, self->state->name, NULL);
        return;
    }

    SEMChord *sem = node(Chord, ast);
    append(self->state, sem);

    NAStackPush(self->stateStack, self->state);
    self->state = StateCreate(sem, CHORD);

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    State *local = self->state;
    self->state = NAStackPop(self->stateStack);
    StateDestroy(local);

    if (inTuplet(self)) {
        SEMTuplet *tuplet = (SEMTuplet *)self->state->node;
        ++tuplet->division;
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

static void parseNoteLength(char *string, NoteLength *noteLength)
{
    char *pc = NACStringDuplicate(string);

    if ('%' == *pc) {
        noteLength->step = atoi(pc + 1);
    }
    else {
        if (*pc) {
            char *digits = pc;
            while (*pc) {
                if ('.' == *pc) {
                    *pc = '\0';
                    ++noteLength->dotCount;
                }

                ++pc;
            }

            noteLength->length = atoi(digits);
        }
    }
}

static State *StateCreate(void *node, const char *name)
{
    State *self = calloc(1, sizeof(State));
    self->node = node;
    self->name = name;
    return self;
}

static void StateDestroy(State *self)
{
    free(self);
}
