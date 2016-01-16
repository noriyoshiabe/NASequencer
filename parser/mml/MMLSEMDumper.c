#include "MMLSEMDumper.h"
#include "MMLSEM.h"
#include <NACString.h>
#include <NALog.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef struct _MMLSEMDumper {
    SEMVisitor visitor;
    Analyzer analyzer;
    int indent;
} MMLSEMDumper;

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

static void dump(MMLSEMDumper *self, void *_node, ...)
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

#define INTEGER(sem, name) #name, NACStringFromInteger(sem->name)
#define FLOAT(sem, name) #name, NACStringFromFloat(sem->name, 2)
#define STRING(sem, name) #name, sem->name ? sem->name : "(null)"
#define CHAR(sem, name) #name, '\0' == sem->name ? "none" : NACStringFromChar(sem->name)
#define BOOL(sem, name) #name, NACStringFromBoolean(sem->name)

static void visitList(void *_self, SEMList *sem)
{
    MMLSEMDumper *self = _self;

    dump(self, sem, NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitTimebase(void *self, SEMTimebase *sem)
{
    dump(self, sem, INTEGER(sem, timebase), NULL);
}

static void visitTitle(void *self, SEMTitle *sem)
{
    dump(self, sem, STRING(sem, title), NULL);
}

static void visitCopyright(void *self, SEMCopyright *sem)
{
    dump(self, sem, STRING(sem, text), NULL);
}

static void visitMarker(void *self, SEMMarker *sem)
{
    dump(self, sem, STRING(sem, text), NULL);
}

static void visitVelocityReverse(void *self, SEMVelocityReverse *sem)
{
    dump(self, sem, NULL);
}

static void visitOctaveReverse(void *self, SEMOctaveReverse *sem)
{
    dump(self, sem, NULL);
}

static void visitChannel(void *self, SEMChannel *sem)
{
    dump(self, sem, INTEGER(sem, number), NULL);
}

static void visitSynth(void *self, SEMSynth *sem)
{
    dump(self, sem, STRING(sem, name), NULL);
}

static void visitBankSelect(void *self, SEMBankSelect *sem)
{
    dump(self, sem, INTEGER(sem, msb), INTEGER(sem, lsb), NULL);
}

static void visitProgramChange(void *self, SEMProgramChange *sem)
{
    dump(self, sem, INTEGER(sem, programNo), NULL);
}

static void visitVolume(void *self, SEMVolume *sem)
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

static void visitPan(void *self, SEMPan *sem)
{
    dump(self, sem, INTEGER(sem, value), NULL);
}

static void visitDetune(void *self, SEMDetune *sem)
{
    dump(self, sem, INTEGER(sem, value), NULL);
}

static void visitTempo(void *self, SEMTempo *sem)
{
    dump(self, sem, FLOAT(sem, tempo), NULL);
}

static void visitNote(void *_self, SEMNote *sem)
{
    MMLSEMDumper *self = _self;
    Node *node = (Node *)sem;

    printf("%*s[%s]", self->indent, "", node->type);

    printf(" baseNote=%s", BaseNote2String(sem->baseNote));
    printf(" accidental=%s", Accidental2String(sem->accidental));
    printf(" %s=%s", INTEGER(sem, length.length));
    printf(" %s=%s", INTEGER(sem, length.dotCount));
    printf(" %s=%s", INTEGER(sem, length.step));
    printf(" %s=%s", STRING(sem, noteString));

    printf(" from %s:%d:%d\n", node->location.filepath, node->location.line, node->location.column);
}

static void visitRest(void *self, SEMRest *sem)
{
    dump(self, sem, INTEGER(sem, length.length), INTEGER(sem, length.dotCount), INTEGER(sem, length.step), NULL);
}

static void visitOctave(void *self, SEMOctave *sem)
{
    dump(self, sem, CHAR(sem, direction), INTEGER(sem, value), NULL);
}

static void visitTransepose(void *self, SEMTranspose *sem)
{
    dump(self, sem, BOOL(sem, relative), INTEGER(sem, value), NULL);
}

static void visitTie(void *self, SEMTie *sem)
{
    dump(self, sem, NULL);
}

static void visitLength(void *self, SEMLength *sem)
{
    dump(self, sem, INTEGER(sem, length), NULL);
}

static void visitGatetime(void *self, SEMGatetime *sem)
{
    dump(self, sem, BOOL(sem, absolute), INTEGER(sem, value), NULL);
}

static void visitVelocity(void *self, SEMVelocity *sem)
{
    dump(self, sem, CHAR(sem, direction), BOOL(sem, absolute), INTEGER(sem, value), NULL);
}

static void visitTuplet(void *_self, SEMTuplet *sem)
{
    MMLSEMDumper *self = _self;

    dump(self, sem, INTEGER(sem, division), INTEGER(sem, length.length), INTEGER(sem, length.dotCount), INTEGER(sem, length.step), NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitTrackChange(void *self, SEMTrackChange *sem)
{
    dump(self, sem, NULL);
}

static void visitRepeat(void *_self, SEMRepeat *sem)
{
    MMLSEMDumper *self = _self;

    dump(self, sem, INTEGER(sem, times), NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitRepeatBreak(void *self, SEMRepeatBreak *sem)
{
    dump(self, sem, NULL);
}

static void visitChord(void *_self, SEMChord *sem)
{
    MMLSEMDumper *self = _self;

    dump(self, sem, NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

Analyzer *MMLSEMDumperCreate(ParseContext *context)
{
    MMLSEMDumper *self = calloc(1, sizeof(MMLSEMDumper));

    self->visitor.visitList = visitList;
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
