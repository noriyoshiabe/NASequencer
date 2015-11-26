#include "NAMidiComposer.h"
#include "NAMidiStatementVisitor.h"

#include <stdlib.h>

#define __Trace__ printf("%s - %s\n", __FILE__, __func__);

struct _NAMidiComposer {
    NAMidiStatementVisitor visitor;
    void *parser;
};

static void visitRoot(void *self, Root *stmt);
static void visitResolution(void *self, Resolution *stmt);
static void visitTitle(void *self, Title *stmt);
static void visitTempo(void *self, Tempo *stmt);
static void visitTime(void *self, Time *stmt);
static void visitKey(void *self, Key *stmt);
static void visitMarker(void *self, Marker *stmt);
static void visitChannel(void *self, Channel *stmt);
static void visitVoice(void *self, Voice *stmt);
static void visitSynth(void *self, Synth *stmt);
static void visitVolume(void *self, Volume *stmt);
static void visitPan(void *self, Pan *stmt);
static void visitChorus(void *self, Chorus *stmt);
static void visitReverb(void *self, Reverb *stmt);
static void visitTranspose(void *self, Transpose *stmt);
static void visitRest(void *self, Rest *stmt);
static void visitNote(void *self, Note *stmt);
static void visitInclude(void *self, Include *stmt);
static void visitPattern(void *self, Pattern *stmt);
static void visitDefine(void *self, Define *stmt);
static void visitContext(void *self, Context *stmt);
static void visitIdentifier(void *self, Identifier *stmt);
static void visitNoteParam(void *self, NoteParam *stmt);

NAMidiComposer *NAMidiComposerCreate(void *parser)
{
    NAMidiComposer *self = calloc(1, sizeof(NAMidiComposer));
    self->parser = parser;

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

void NAMidiComposerDestroy(NAMidiComposer *self)
{
    free(self);
}

static void visitRoot(void *_self, Root *stmt)
{ __Trace__
    NAMidiComposer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(stmt->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitResolution(void *_self, Resolution *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}

static void visitTitle(void *_self, Title *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}

static void visitTempo(void *_self, Tempo *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}

static void visitTime(void *_self, Time *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}

static void visitKey(void *_self, Key *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}

static void visitMarker(void *_self, Marker *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}

static void visitChannel(void *_self, Channel *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}

static void visitVoice(void *_self, Voice *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}

static void visitSynth(void *_self, Synth *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}

static void visitVolume(void *_self, Volume *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}

static void visitPan(void *_self, Pan *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}

static void visitChorus(void *_self, Chorus *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}

static void visitReverb(void *_self, Reverb *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}

static void visitTranspose(void *_self, Transpose *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}

static void visitRest(void *_self, Rest *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}

static void visitNote(void *_self, Note *stmt)
{ __Trace__
    NAMidiComposer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(stmt->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitInclude(void *_self, Include *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}

static void visitPattern(void *_self, Pattern *stmt)
{ __Trace__
    NAMidiComposer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(stmt->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitDefine(void *_self, Define *stmt)
{ __Trace__
    NAMidiComposer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(stmt->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitContext(void *_self, Context *stmt)
{ __Trace__
    NAMidiComposer *self = _self;

    NAIterator *iterator = NAArrayGetIterator(stmt->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitIdentifier(void *_self, Identifier *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}

static void visitNoteParam(void *_self, NoteParam *stmt)
{ __Trace__
    NAMidiComposer *self = _self;
}
