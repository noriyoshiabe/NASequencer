#include "MMLASTAnalyzer.h"
#include "MMLParser.h"
#include "MMLAST.h"
#include "MMLSEM.h"
#include "NALog.h"

#include <stdlib.h>

#define node(type, ast) MMLSEM##type##Create(&ast->node.location)
#define append(list, sem) NAArrayAppend(list->node.children, sem)
#define appendError(self, ast, ...) self->context->appendError(self->context, &ast->node.location, __VA_ARGS__)

#define isValidRange(v, from, to) (from <= v && v <= to)

typedef struct _MMLASTAnalyzer {
    ASTVisitor visitor;
    Analyzer analyzer;
    ParseContext *context;
} MMLASTAnalyzer;

static Node *process(void *_self, Node *node)
{
    MMLASTAnalyzer *self = _self;

    SEMList *list = MMLSEMListCreate(NULL);

    node->accept(node, self);

    //return (Node *)list;
    NodeRelease(list);
    return NULL;
}

static void destroy(void *_self)
{
    MMLASTAnalyzer *self = _self;
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

static void visitTimebase(void *self, ASTTimebase *ast)
{
    __Trace__
}

static void visitTitle(void *self, ASTTitle *ast)
{
    __Trace__
}

static void visitCopyright(void *self, ASTCopyright *ast)
{
    __Trace__
}

static void visitMarker(void *self, ASTMarker *ast)
{
    __Trace__
}

static void visitVelocityReverse(void *self, ASTVelocityReverse *ast)
{
    __Trace__
}

static void visitOctaveReverse(void *self, ASTOctaveReverse *ast)
{
    __Trace__
}

static void visitChannel(void *self, ASTChannel *ast)
{
    __Trace__
}

static void visitSynth(void *self, ASTSynth *ast)
{
    __Trace__
}

static void visitBankSelect(void *self, ASTBankSelect *ast)
{
    __Trace__
}

static void visitProgramChange(void *self, ASTProgramChange *ast)
{
    __Trace__
}

static void visitVolume(void *self, ASTVolume *ast)
{
    __Trace__
}

static void visitChorus(void *self, ASTChorus *ast)
{
    __Trace__
}

static void visitReverb(void *self, ASTReverb *ast)
{
    __Trace__
}

static void visitExpression(void *self, ASTExpression *ast)
{
    __Trace__
}

static void visitPan(void *self, ASTPan *ast)
{
    __Trace__
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

    return &self->analyzer;
}
