#include "ABCASTAnalyzer.h"
#include "ABCParser.h"
#include "ABCAST.h"
#include "ABCSEM.h"

#include <stdlib.h>
#include <string.h>

#define node(type, ast) ABCSEM##type##Create(&ast->node.location)
#define append(list, sem) NAArrayAppend(list->node.children, sem)
#define appendError(self, ast, ...) self->context->appendError(self->context, &ast->node.location, __VA_ARGS__)

#define __Trace__ printf("-- %s:%s - %d\n", __FILE__, __func__, __LINE__);

typedef enum {
    FileHeader,
    TuneHeader,
    TuneBody,
} State;

static inline const char *State2String(State state)
{
#define CASE(state) case state: return #state;
    switch (state) {
    CASE(FileHeader);
    CASE(TuneHeader);
    CASE(TuneBody);
    }
#undef CASE
}

typedef struct _ABCASTAnalyzer {
    ASTVisitor visitor;
    Analyzer analyzer;

    ParseContext *context;
    State state;

    SEMFile *file;
    SEMTune *tune;
    SEMKey *key;
} ABCASTAnalyzer;

static Node *process(void *_self, Node *node)
{
    ABCASTAnalyzer *self = _self;

    SEMFile *file = ABCSEMFileCreate(NULL);
    file->node.children = NAArrayCreate(4, NADescriptionAddress);

    self->file = file;
    node->accept(node, self);

    return (Node *)file;
}

static void destroy(void *_self)
{
    ABCASTAnalyzer *self = _self;
    free(self);
}

static void visitRoot(void *self, ASTRoot *ast)
{
    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitFileIdentification(void *self, ASTFileIdentification *ast)
{
}

static void visitStringInformation(void *self, ASTStringInformation *ast)
{
}

static void visitReferenceNumber(void *_self, ASTReferenceNumber *ast)
{
    ABCASTAnalyzer *self = _self;

    SEMTune *tune = node(Tune, ast);
    tune->number = ast->number;

    append(self->file, tune);
    self->tune = tune;

    self->state = TuneHeader;
}

static void visitTitle(void *_self, ASTTitle *ast)
{
    ABCASTAnalyzer *self = _self;
    
    if (!self->tune) {
        appendError(self, ast, ABCParseErrorIllegalStateWithTitle, State2String(self->state), NULL);
        return;
    }

    NAArrayAppend(self->tune->titleList, strdup(ast->title));
}

static void visitKey(void *_self, ASTKey *ast)
{
    ABCASTAnalyzer *self = _self;

    if (!self->tune) {
        appendError(self, ast, ABCParseErrorIllegalStateWithKey, State2String(self->state), NULL);
        return;
    }

    self->state = TuneBody;

    SEMKey *key = node(Key, ast);
    self->key = key;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    // TODO
    BaseNote baseNote = BaseNote_C;
    bool sharp = false;
    bool flat = true;
    Mode mode = ModeMajor;
    key->noteTable = NoteTableCreate(baseNote, sharp, flat, ModeMajor);

    append(self->tune, key);
}

static void visitKeyParam(void *self, ASTKeyParam *ast)
{
    __Trace__
}

static void visitMeter(void *self, ASTMeter *ast)
{
    __Trace__
}

static void visitUnitNoteLength(void *self, ASTUnitNoteLength *ast)
{
    __Trace__
}

static void visitTempo(void *self, ASTTempo *ast)
{
    __Trace__

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitTempoParam(void *self, ASTTempoParam *ast)
{
    __Trace__
}

static void visitParts(void *self, ASTParts *ast)
{
    __Trace__
}

static void visitInstCharSet(void *self, ASTInstCharSet *ast)
{
    __Trace__
}

static void visitInstVersion(void *self, ASTInstVersion *ast)
{
    __Trace__
}

static void visitInstInclude(void *self, ASTInstInclude *ast)
{
    __Trace__

    if (ast->root) {
        ast->root->accept(ast->root, self);
    }
}

static void visitInstCreator(void *self, ASTInstCreator *ast)
{
    __Trace__
}

static void visitInstLineBreak(void *self, ASTInstLineBreak *ast)
{
    __Trace__
}

static void visitInstDecoration(void *self, ASTInstDecoration *ast)
{
    __Trace__
}

static void visitMacro(void *self, ASTMacro *ast)
{
    __Trace__
}

static void visitSymbolLine(void *self, ASTSymbolLine *ast)
{
    __Trace__
}

static void visitRedefinableSymbol(void *self, ASTRedefinableSymbol *ast)
{
    __Trace__
}

static void visitContinuation(void *self, ASTContinuation *ast)
{
    __Trace__
}

static void visitVoice(void *self, ASTVoice *ast)
{
    __Trace__

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitVoiceParam(void *self, ASTVoiceParam *ast)
{
    __Trace__
}

static void visitTuneBody(void *self, ASTTuneBody *ast)
{
    __Trace__

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitLineBreak(void *self, ASTLineBreak *ast)
{
    __Trace__
}

static void visitAnnotation(void *self, ASTAnnotation *ast)
{
    __Trace__
}

static void visitDecoration(void *self, ASTDecoration *ast)
{
    __Trace__
}

static void visitNote(void *self, ASTNote *ast)
{
    __Trace__
}

static void visitBrokenRhythm(void *self, ASTBrokenRhythm *ast)
{
    __Trace__
}

static void visitRest(void *self, ASTRest *ast)
{
    __Trace__
}

static void visitRepeatBar(void *self, ASTRepeatBar *ast)
{
    __Trace__
}

static void visitTie(void *self, ASTTie *ast)
{
    __Trace__
}

static void visitSlur(void *self, ASTSlur *ast)
{
    __Trace__
}

static void visitDot(void *self, ASTDot *ast)
{
    __Trace__
}

static void visitGraceNote(void *self, ASTGraceNote *ast)
{
    __Trace__

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitTuplet(void *self, ASTTuplet *ast)
{
    __Trace__
}

static void visitChord(void *self, ASTChord *ast)
{
    __Trace__

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitOverlay(void *self, ASTOverlay *ast)
{
    __Trace__
}

static void visitEmptyLine(void *self, ASTEmptyLine *ast)
{
    __Trace__
}

static void visitMidi(void *self, ASTMidi *ast)
{
    __Trace__

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitMidiParam(void *self, ASTMidiParam *ast)
{
    __Trace__
}

static void visitPropagateAccidental(void *self, ASTPropagateAccidental *ast)
{
    __Trace__
}


Analyzer *ABCASTAnalyzerCreate(ParseContext *context)
{
    ABCASTAnalyzer *self = calloc(1, sizeof(ABCASTAnalyzer));

    self->visitor.visitRoot = visitRoot;
    self->visitor.visitFileIdentification = visitFileIdentification;
    self->visitor.visitStringInformation = visitStringInformation;
    self->visitor.visitTitle = visitTitle;
    self->visitor.visitReferenceNumber = visitReferenceNumber;
    self->visitor.visitKey = visitKey;
    self->visitor.visitKeyParam = visitKeyParam;
    self->visitor.visitMeter = visitMeter;
    self->visitor.visitUnitNoteLength = visitUnitNoteLength;
    self->visitor.visitTempo = visitTempo;
    self->visitor.visitTempoParam = visitTempoParam;
    self->visitor.visitParts = visitParts;
    self->visitor.visitInstCharSet = visitInstCharSet;
    self->visitor.visitInstVersion = visitInstVersion;
    self->visitor.visitInstInclude = visitInstInclude;
    self->visitor.visitInstCreator = visitInstCreator;
    self->visitor.visitInstLineBreak = visitInstLineBreak;
    self->visitor.visitInstDecoration = visitInstDecoration;
    self->visitor.visitSymbolLine = visitSymbolLine;
    self->visitor.visitMacro = visitMacro;
    self->visitor.visitContinuation = visitContinuation;
    self->visitor.visitRedefinableSymbol = visitRedefinableSymbol;
    self->visitor.visitVoice = visitVoice;
    self->visitor.visitVoiceParam = visitVoiceParam;
    self->visitor.visitTuneBody = visitTuneBody;
    self->visitor.visitLineBreak = visitLineBreak;
    self->visitor.visitAnnotation = visitAnnotation;
    self->visitor.visitDecoration = visitDecoration;
    self->visitor.visitNote = visitNote;
    self->visitor.visitBrokenRhythm = visitBrokenRhythm;
    self->visitor.visitRest = visitRest;
    self->visitor.visitRepeatBar = visitRepeatBar;
    self->visitor.visitTie = visitTie;
    self->visitor.visitSlur = visitSlur;
    self->visitor.visitDot = visitDot;
    self->visitor.visitGraceNote = visitGraceNote;
    self->visitor.visitTuplet = visitTuplet;
    self->visitor.visitChord = visitChord;
    self->visitor.visitOverlay = visitOverlay;
    self->visitor.visitEmptyLine = visitEmptyLine;
    self->visitor.visitMidi = visitMidi;
    self->visitor.visitMidiParam = visitMidiParam;
    self->visitor.visitPropagateAccidental = visitPropagateAccidental;

    self->analyzer.process = process;
    self->analyzer.destroy = destroy;
    self->analyzer.self = self;

    self->context = context;

    return &self->analyzer;
}
