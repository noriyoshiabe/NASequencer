#include "NAMidiSEMDumper.h"
#include "NAMidiSEM.h"
#include "NACString.h"
#include <NALog.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <alloca.h>

typedef struct _NAMidiSEMDumper {
    SEMVisitor visitor;
    Analyzer analyzer;
    int indent;
} NAMidiSEMDumper;

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

static void dump(NAMidiSEMDumper *self, void *_node, ...)
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

    printf(" from %s:%d:%d\n", node->location.filepath, node->location.line, node->location.column);
}

static char *array2String(NAArray *array, char *buffer)
{
    buffer[0] = '\0';

    NAIterator *iterator = NAArrayGetIterator(array);
    while (iterator->hasNext(iterator)) {
        if (buffer[0]) {
            strcat(buffer, ",");
        }
        strcat(buffer, iterator->next(iterator));
    }

    return buffer;
}

#define INTEGER(sem, name) #name, NACStringFromInteger(sem->name)
#define FLOAT(sem, name) #name, NACStringFromFloat(sem->name, 2)
#define STRING(sem, name) #name, sem->name ? sem->name : "(null)"
#define STRING_ARRAY(sem, name) #name, array2String(sem->name, alloca(1024))

static void visitList(void *_self, SEMList *sem)
{
    NAMidiSEMDumper *self = _self;

    dump(self, sem, STRING(sem, identifier), NULL);
    self->indent += 4;

    NAIterator *iterator;
    
    iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    iterator = NAMapGetIterator(sem->patternMap);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        Node *node = entry->value;
        printf("%*s---------- pattern ----------\n", self->indent, "");
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitResolution(void *self, SEMResolution *sem)
{
    dump(self, sem, INTEGER(sem, resolution), NULL);
}

static void visitTitle(void *self, SEMTitle *sem)
{
    dump(self, sem, STRING(sem, title), NULL);
}

static void visitCopyright(void *self, SEMCopyright *sem)
{
    dump(self, sem, STRING(sem, text), NULL);
}

static void visitTempo(void *self, SEMTempo *sem)
{
    dump(self, sem, FLOAT(sem, tempo), NULL);
}

static void visitTime(void *self, SEMTime *sem)
{
    dump(self, sem, INTEGER(sem, numerator), INTEGER(sem, denominator), NULL);
}

static void visitKey(void *_self, SEMKey *sem)
{
    NAMidiSEMDumper *self = _self;
    dump(self, sem, NULL);
    NoteTableDump(sem->noteTable, self->indent + 4);
}

static void visitMarker(void *self, SEMMarker *sem)
{
    dump(self, sem, STRING(sem, text), NULL);
}

static void visitChannel(void *self, SEMChannel *sem)
{
    dump(self, sem, INTEGER(sem, number), NULL);
}

static void visitVelocity(void *self, SEMVelocity *sem)
{
    dump(self, sem, INTEGER(sem, value), NULL);
}

static void visitGatetime(void *self, SEMGatetime *sem)
{
    dump(self, sem, INTEGER(sem, absolute), INTEGER(sem, value), NULL);
}

static void visitVoice(void *self, SEMVoice *sem)
{
    dump(self, sem, INTEGER(sem, msb), INTEGER(sem, lsb), INTEGER(sem, programNo), NULL);
}

static void visitSynth(void *self, SEMSynth *sem)
{
    dump(self, sem, STRING(sem, name), NULL);
}

static void visitVolume(void *self, SEMVolume *sem)
{
    dump(self, sem, INTEGER(sem, value), NULL);
}

static void visitPan(void *self, SEMPan *sem)
{
    dump(self, sem, INTEGER(sem, value), NULL);
}

static void visitChorus(void *self, SEMChorus *sem)
{
    dump(self, sem, INTEGER(sem, value), NULL);
}

static void visitReverb(void *self, SEMReverb *sem)
{
    dump(self, sem, INTEGER(sem, value), NULL);
}

static void visitExpression(void *self, SEMExpression *sem)
{
    dump(self, sem, INTEGER(sem, value), NULL);
}

static void visitDetune(void *self, SEMDetune *sem)
{
    dump(self, sem, INTEGER(sem, value), NULL);
}

static void visitTranspose(void *self, SEMTranspose *sem)
{
    dump(self, sem, INTEGER(sem, value), NULL);
}

static void visitStep(void *self, SEMStep *sem)
{
    dump(self, sem, INTEGER(sem, step), NULL);
}

static void visitNote(void *_self, SEMNote *sem)
{
    NAMidiSEMDumper *self = _self;
    Node *node = (Node *)sem;

    printf("%*s[%s]", self->indent, "", node->type);

    printf(" baseNote=%s", BaseNote2String(sem->baseNote));
    printf(" accidental=%s", Accidental2String(sem->accidental));
    printf(" %s=%s", INTEGER(sem, octave));
    printf(" %s=%s", INTEGER(sem, gatetime));
    printf(" %s=%s", INTEGER(sem, velocity));
    printf(" %s=%s", STRING(sem, noteString));

    printf(" from %s:%d:%d\n", node->location.filepath, node->location.line, node->location.column);
}

static void visitPattern(void *self, SEMPattern *sem)
{
    dump(self, sem, STRING(sem, identifier), STRING_ARRAY(sem, ctxIdList), NULL);
}

static void visitContext(void *_self, SEMContext *sem)
{
    NAMidiSEMDumper *self = _self;

    dump(self, sem, STRING_ARRAY(sem, ctxIdList), NULL);

    self->indent += 4;
    sem->list->node.accept(sem->list, self);
    self->indent -= 4;
}

Analyzer *NAMidiSEMDumperCreate(ParseContext *context)
{
    NAMidiSEMDumper *self = calloc(1, sizeof(NAMidiSEMDumper));
    
    self->visitor.visitList = visitList;
    self->visitor.visitResolution = visitResolution;
    self->visitor.visitTitle = visitTitle;
    self->visitor.visitCopyright = visitCopyright;
    self->visitor.visitTempo = visitTempo;
    self->visitor.visitTime = visitTime;
    self->visitor.visitKey = visitKey;
    self->visitor.visitMarker = visitMarker;
    self->visitor.visitChannel = visitChannel;
    self->visitor.visitVelocity = visitVelocity;
    self->visitor.visitGatetime = visitGatetime;
    self->visitor.visitVoice = visitVoice;
    self->visitor.visitSynth = visitSynth;
    self->visitor.visitVolume = visitVolume;
    self->visitor.visitPan = visitPan;
    self->visitor.visitChorus = visitChorus;
    self->visitor.visitReverb = visitReverb;
    self->visitor.visitExpression = visitExpression;
    self->visitor.visitDetune = visitDetune;
    self->visitor.visitTranspose = visitTranspose;
    self->visitor.visitStep = visitStep;
    self->visitor.visitNote = visitNote;
    self->visitor.visitPattern = visitPattern;
    self->visitor.visitContext = visitContext;

    self->analyzer.process = process;
    self->analyzer.destroy = destroy;
    self->analyzer.self = self;

    return &self->analyzer;
}
