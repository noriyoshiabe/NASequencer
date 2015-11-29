#include "NAMidiAST.h"
#include "NAMidiASTParser.h"

#include <stdlib.h>

#define __Trace__ printf("%s - %s\n", __FILE__, __func__);

struct _NAMidiASTParser {
    ASTVisitor visitor;
    void *parser;
};

static void visitRoot(void *self, ASTRoot *ast);
static void visitResolution(void *self, ASTResolution *ast);
static void visitTitle(void *self, ASTTitle *ast);
static void visitTempo(void *self, ASTTempo *ast);
static void visitTime(void *self, ASTTime *ast);
static void visitKey(void *self, ASTKey *ast);
static void visitMarker(void *self, ASTMarker *ast);
static void visitChannel(void *self, ASTChannel *ast);
static void visitVoice(void *self, ASTVoice *ast);
static void visitSynth(void *self, ASTSynth *ast);
static void visitVolume(void *self, ASTVolume *ast);
static void visitPan(void *self, ASTPan *ast);
static void visitChorus(void *self, ASTChorus *ast);
static void visitReverb(void *self, ASTReverb *ast);
static void visitTranspose(void *self, ASTTranspose *ast);
static void visitRest(void *self, ASTRest *ast);
static void visitNote(void *self, ASTNote *ast);
static void visitInclude(void *self, ASTInclude *ast);
static void visitPattern(void *self, ASTPattern *ast);
static void visitDefine(void *self, ASTDefine *ast);
static void visitContext(void *self, ASTContext *ast);
static void visitIdentifier(void *self, ASTIdentifier *ast);
static void visitNoteParam(void *self, ASTNoteParam *ast);

NAMidiASTParser *NAMidiASTParserCreate()
{
    NAMidiASTParser *self = calloc(1, sizeof(NAMidiASTParser));

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

    return self;
}

void NAMidiASTParserDestroy(NAMidiASTParser *self)
{
    free(self);
}

static void visitRoot(void *_self, ASTRoot *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitResolution(void *_self, ASTResolution *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitTitle(void *_self, ASTTitle *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitTempo(void *_self, ASTTempo *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitTime(void *_self, ASTTime *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitKey(void *_self, ASTKey *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitMarker(void *_self, ASTMarker *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitChannel(void *_self, ASTChannel *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitVoice(void *_self, ASTVoice *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitSynth(void *_self, ASTSynth *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitVolume(void *_self, ASTVolume *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitPan(void *_self, ASTPan *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitChorus(void *_self, ASTChorus *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitReverb(void *_self, ASTReverb *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitTranspose(void *_self, ASTTranspose *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitRest(void *_self, ASTRest *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitNote(void *_self, ASTNote *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitInclude(void *_self, ASTInclude *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitPattern(void *_self, ASTPattern *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitDefine(void *_self, ASTDefine *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitContext(void *_self, ASTContext *ast)
{ __Trace__
    NAMidiASTParser *self = _self;

    NAIterator *iterator = NAArrayGetIterator(ast->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitIdentifier(void *_self, ASTIdentifier *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}

static void visitNoteParam(void *_self, ASTNoteParam *ast)
{ __Trace__
    NAMidiASTParser *self = _self;
}
