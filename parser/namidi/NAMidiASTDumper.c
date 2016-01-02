#include "NAMidiASTDumper.h"
#include "NAMidiAST.h"
#include <NACString.h>
#include <NALog.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct _NAMidiASTDumper {
    ASTVisitor visitor;
    Analyzer analyzer;
    int indent;
} NAMidiASTDumper;

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

static void dump(NAMidiASTDumper *self, void *_node, ...)
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

static void visitRoot(void *_self, ASTRoot *ast)
{
    NAMidiASTDumper *self = _self;

    dump(self, ast, NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitResolution(void *self, ASTResolution *ast)
{
    dump(self, ast, INTEGER(ast, resolution), NULL);
}

static void visitTitle(void *self, ASTTitle *ast)
{
    dump(self, ast, STRING(ast, title), NULL);
}

static void visitTempo(void *self, ASTTempo *ast)
{
    dump(self, ast, FLOAT(ast, tempo), NULL);
}

static void visitTime(void *self, ASTTime *ast)
{
    dump(self, ast, INTEGER(ast, numerator), INTEGER(ast, denominator), NULL);
}

static void visitKey(void *self, ASTKey *ast)
{
    dump(self, ast, STRING(ast, keyString), NULL);
}

static void visitMarker(void *self, ASTMarker *ast)
{
    dump(self, ast, STRING(ast, text), NULL);
}

static void visitChannel(void *self, ASTChannel *ast)
{
    dump(self, ast, INTEGER(ast, number), NULL);
}

static void visitVoice(void *self, ASTVoice *ast)
{
    dump(self, ast, INTEGER(ast, msb), INTEGER(ast, lsb), INTEGER(ast, programNo), NULL);
}

static void visitSynth(void *self, ASTSynth *ast)
{
    dump(self, ast, STRING(ast, name), NULL);
}

static void visitVolume(void *self, ASTVolume *ast)
{
    dump(self, ast, INTEGER(ast, value), NULL);
}

static void visitPan(void *self, ASTPan *ast)
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

static void visitTranspose(void *self, ASTTranspose *ast)
{
    dump(self, ast, INTEGER(ast, value), NULL);
}

static void visitStep(void *self, ASTStep *ast)
{
    dump(self, ast, INTEGER(ast, step), NULL);
}

static void visitNote(void *_self, ASTNote *ast)
{
    NAMidiASTDumper *self = _self;

    dump(self, ast, STRING(ast, noteString), NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitInclude(void *_self, ASTInclude *ast)
{
    NAMidiASTDumper *self = _self;

    dump(self, ast, STRING(ast, filepath), NULL);
    self->indent += 4;

    if (ast->root) {
        ast->root->accept(ast->root, self);
    }

    self->indent -= 4;
}

static void visitPattern(void *_self, ASTPattern *ast)
{
    NAMidiASTDumper *self = _self;

    dump(self, ast, STRING(ast, identifier), NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitDefine(void *_self, ASTDefine *ast)
{
    NAMidiASTDumper *self = _self;

    dump(self, ast, STRING(ast, identifier), NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitContext(void *_self, ASTContext *ast)
{
    NAMidiASTDumper *self = _self;

    dump(self, ast, NULL);
    self->indent += 4;

    NAIterator *iterator;

    iterator = NAArrayGetIterator(ast->ctxIdList);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitIdentifier(void *self, ASTIdentifier *ast)
{
    dump(self, ast, STRING(ast, idString), NULL);
}

static void visitNoteParam(void *self, ASTNoteParam *ast)
{
    dump(self, ast, INTEGER(ast, value), NULL);
}

Analyzer *NAMidiASTDumperCreate(ParseContext *context)
{
    NAMidiASTDumper *self = calloc(1, sizeof(NAMidiASTDumper));

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
    self->visitor.visitStep = visitStep;
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

    return &self->analyzer;
}
