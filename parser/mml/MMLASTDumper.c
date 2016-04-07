#include "MMLASTDumper.h"
#include "MMLAST.h"
#include <NACString.h>
#include <NALog.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct _MMLASTDumper {
    ASTVisitor visitor;
    Analyzer analyzer;
    int indent;
} MMLASTDumper;

static Node *process(void *self, Node *node)
{
    if (__IsDebug__) {
        node->accept(node, self);
    }
    return NodeRetain(node);
}

static void destroy(void *self)
{
    free(self);
}

static void dump(MMLASTDumper *self, void *_node, ...)
{
    Node *node = _node;

    printf("%*s", self->indent, "");
    printf("[%s]", node->type);

    va_list argList;
    va_start(argList, _node);

    const char *str;
    int i = 0;
    while ((str = va_arg(argList, const char *))) {
        printf("%c", 0 == i % 2 ? ' ' : '=');
        printf("%s", str);
        ++i;
    }

    va_end(argList);

    printf(" at %s:%d:%d\n", node->location.filepath, node->location.line, node->location.column);
}

#define INTEGER(ast, name) #name, NACStringFromInteger(ast->name)
#define FLOAT(ast, name) #name, NACStringFromFloat(ast->name, 2)
#define STRING(ast, name) #name, ast->name ? ast->name : "(null)"
#define CHAR(ast, name) #name, '\0' == ast->name ? "none" : NACStringFromChar(ast->name)
#define BOOL(ast, name) #name, NACStringFromBoolean(ast->name)

static void visitRoot(void *_self, ASTRoot *ast)
{
    MMLASTDumper *self = _self;

    dump(self, ast, NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitTimebase(void *self, ASTTimebase *ast)
{
    dump(self, ast, INTEGER(ast, timebase), NULL);
}

static void visitTitle(void *self, ASTTitle *ast)
{
    dump(self, ast, STRING(ast, title), NULL);
}

static void visitCopyright(void *self, ASTCopyright *ast)
{
    dump(self, ast, STRING(ast, text), NULL);
}

static void visitMarker(void *self, ASTMarker *ast)
{
    dump(self, ast, STRING(ast, text), NULL);
}

static void visitVelocityReverse(void *self, ASTVelocityReverse *ast)
{
    dump(self, ast, NULL);
}

static void visitOctaveReverse(void *self, ASTOctaveReverse *ast)
{
    dump(self, ast, NULL);
}

static void visitChannel(void *self, ASTChannel *ast)
{
    dump(self, ast, INTEGER(ast, number), NULL);
}

static void visitSynth(void *self, ASTSynth *ast)
{
    dump(self, ast, STRING(ast, name), NULL);
}

static void visitBankSelect(void *self, ASTBankSelect *ast)
{
    dump(self, ast, INTEGER(ast, bankNo), NULL);
}

static void visitProgramChange(void *self, ASTProgramChange *ast)
{
    dump(self, ast, INTEGER(ast, programNo), NULL);
}

static void visitVolume(void *self, ASTVolume *ast)
{
    dump(self, ast, INTEGER(ast, value), NULL);
}

static void visitChorus(void *self, ASTChorus *ast)
{
    dump(self, ast, INTEGER(ast, value), NULL);
}

static void visitReverb(void *self, ASTReverb *ast)
{
    dump(self, ast, INTEGER(ast, value), NULL);
}

static void visitExpression(void *self, ASTExpression *ast)
{
    dump(self, ast, INTEGER(ast, value), NULL);
}

static void visitPan(void *self, ASTPan *ast)
{
    dump(self, ast, INTEGER(ast, value), NULL);
}

static void visitPitch(void *self, ASTPitch *ast)
{
    dump(self, ast, INTEGER(ast, value), BOOL(ast, coarse), NULL);
}

static void visitDetune(void *self, ASTDetune *ast)
{
    dump(self, ast, INTEGER(ast, value), NULL);
}

static void visitPitchSense(void *self, ASTPitchSense *ast)
{
    dump(self, ast, INTEGER(ast, value), NULL);
}

static void visitSustain(void *self, ASTSustain *ast)
{
    dump(self, ast, INTEGER(ast, value), NULL);
}

static void visitTempo(void *self, ASTTempo *ast)
{
    dump(self, ast, FLOAT(ast, tempo), NULL);
}

static void visitTime(void *self, ASTTime *ast)
{
    dump(self, ast, INTEGER(ast, numerator), INTEGER(ast, denominator), NULL);
}

static void visitNote(void *self, ASTNote *ast)
{
    dump(self, ast, STRING(ast, noteString), NULL);
}

static void visitRest(void *self, ASTRest *ast)
{
    dump(self, ast, STRING(ast, restString), NULL);
}

static void visitOctave(void *self, ASTOctave *ast)
{
    dump(self, ast, CHAR(ast, direction), INTEGER(ast, value), NULL);
}

static void visitTransepose(void *self, ASTTransepose *ast)
{
    dump(self, ast, BOOL(ast, relative), INTEGER(ast, value), NULL);
}

static void visitTie(void *self, ASTTie *ast)
{
    dump(self, ast, NULL);
}

static void visitLength(void *self, ASTLength *ast)
{
    dump(self, ast, INTEGER(ast, length), NULL);
}

static void visitGatetime(void *self, ASTGatetime *ast)
{
    dump(self, ast, BOOL(ast, absolute), INTEGER(ast, value), NULL);
}

static void visitVelocity(void *self, ASTVelocity *ast)
{
    dump(self, ast, CHAR(ast, direction), BOOL(ast, absolute), INTEGER(ast, value), NULL);
}

static void visitTuplet(void *_self, ASTTuplet *ast)
{
    MMLASTDumper *self = _self;

    dump(self, ast, STRING(ast, lengthString), NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitTrackChange(void *self, ASTTrackChange *ast)
{
    dump(self, ast, NULL);
}

static void visitRepeat(void *_self, ASTRepeat *ast)
{
    MMLASTDumper *self = _self;

    dump(self, ast, INTEGER(ast, times), NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitRepeatBreak(void *self, ASTRepeatBreak *ast)
{
    dump(self, ast, NULL);
}

static void visitChord(void *_self, ASTChord *ast)
{
    MMLASTDumper *self = _self;

    dump(self, ast, NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

Analyzer *MMLASTDumperCreate(ParseContext *context)
{
    MMLASTDumper *self = calloc(1, sizeof(MMLASTDumper));

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
    self->visitor.visitPitch = visitPitch;
    self->visitor.visitDetune = visitDetune;
    self->visitor.visitPitchSense = visitPitchSense;
    self->visitor.visitSustain = visitSustain;
    self->visitor.visitTempo = visitTempo;
    self->visitor.visitTime = visitTime;
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
