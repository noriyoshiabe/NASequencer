#include "NAMidiSEMParser.h"
#include "NAMidiSEM.h"

#include <stdio.h>
#include <stdlib.h>

#define __Trace__ printf("%s - %s\n", __FILE__, __func__);

struct _NAMidiSEMParser {
    SEMVisitor visitor;
    void *parser;
};

static void visitList(void *self, SEMList *sem);
static void visitResolution(void *self, SEMResolution *sem);
static void visitTitle(void *self, SEMTitle *sem);
static void visitTempo(void *self, SEMTempo *sem);
static void visitTime(void *self, SEMTime *sem);
static void visitKey(void *self, SEMKey *sem);
static void visitMarker(void *self, SEMMarker *sem);
static void visitChannel(void *self, SEMChannel *sem);
static void visitVoice(void *self, SEMVoice *sem);
static void visitSynth(void *self, SEMSynth *sem);
static void visitVolume(void *self, SEMVolume *sem);
static void visitPan(void *self, SEMPan *sem);
static void visitChorus(void *self, SEMChorus *sem);
static void visitReverb(void *self, SEMReverb *sem);
static void visitTranspose(void *self, SEMTranspose *sem);
static void visitRest(void *self, SEMRest *sem);
static void visitNote(void *self, SEMNote *sem);
static void visitPattern(void *self, SEMPattern *sem);
static void visitContext(void *self, SEMContext *sem);

NAMidiSEMParser *NAMidiSEMParserCreate(void *parser)
{
    NAMidiSEMParser *self = calloc(1, sizeof(NAMidiSEMParser));
    
    self->visitor.visitList = visitList;
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
    self->visitor.visitPattern = visitPattern;
    self->visitor.visitContext = visitContext;

    self->parser = parser;

    return self;
}

void NAMidiSEMParserDestroy(NAMidiSEMParser *self)
{
    free(self);
}


void *NAMidiSEMParserBuildSequence(NAMidiSEMParser *self, Node *node)
{
    node->accept(node, self);
    return NULL;
}

static void visitList(void *self, SEMList *sem)
{ __Trace__

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }
}

static void visitResolution(void *self, SEMResolution *sem)
{ __Trace__
}

static void visitTitle(void *self, SEMTitle *sem)
{ __Trace__
}

static void visitTempo(void *self, SEMTempo *sem)
{ __Trace__
}

static void visitTime(void *self, SEMTime *sem)
{ __Trace__
}

static void visitKey(void *self, SEMKey *sem)
{ __Trace__
}

static void visitMarker(void *self, SEMMarker *sem)
{ __Trace__
}

static void visitChannel(void *self, SEMChannel *sem)
{ __Trace__
}

static void visitVoice(void *self, SEMVoice *sem)
{ __Trace__
}

static void visitSynth(void *self, SEMSynth *sem)
{ __Trace__
}

static void visitVolume(void *self, SEMVolume *sem)
{ __Trace__
}

static void visitPan(void *self, SEMPan *sem)
{ __Trace__
}

static void visitChorus(void *self, SEMChorus *sem)
{ __Trace__
}

static void visitReverb(void *self, SEMReverb *sem)
{ __Trace__
}

static void visitTranspose(void *self, SEMTranspose *sem)
{ __Trace__
}

static void visitRest(void *self, SEMRest *sem)
{ __Trace__
}

static void visitNote(void *self, SEMNote *sem)
{ __Trace__
}

static void visitPattern(void *self, SEMPattern *sem)
{ __Trace__
}

static void visitContext(void *self, SEMContext *sem)
{ __Trace__
}

