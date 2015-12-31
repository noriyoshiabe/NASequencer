#include "ABCSEMDumper.h"
#include "ABCSEM.h"
#include "NACString.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <alloca.h>

typedef struct _ABCSEMDumper {
    SEMVisitor visitor;
    Analyzer analyzer;
    int indent;
} ABCSEMDumper;

static Node *process(void *self, Node *node)
{
    node->accept(node, self);
    //return NodeRetain(node);
    return NULL;
}

static void destroy(void *self)
{
    free(self);
}

static void dump(ABCSEMDumper *self, void *_node, ...)
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
#define CHAR(ast, name) #name, '\n' == ast->name ? "\\n" : NACStringFromChar(ast->name)
#define BOOL(ast, name) #name, NACStringFromBoolean(ast->name)
#define STRING(sem, name) #name, sem->name ? sem->name : "(null)"
#define STRING_ARRAY(sem, name) #name, array2String(sem->name, alloca(1024))

static void visitFile(void *_self, SEMFile *sem)
{
    ABCSEMDumper *self = _self;

    dump(self, sem, NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitTune(void *_self, SEMTune *sem)
{
    ABCSEMDumper *self = _self;

    dump(self, sem, INTEGER(sem, number), STRING_ARRAY(sem, titleList), STRING(sem, partSequence), NULL);
    self->indent += 4;

    NAIterator *iterator;
   
    iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    iterator = NAMapGetIterator(sem->partMap);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        Node *node = entry->value;
        printf("%*s---------- part ----------\n", self->indent, "");
        node->accept(node, self);
    }

    iterator = NAMapGetIterator(sem->voiceMap);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        Node *node = entry->value;
        printf("%*s---------- voice ----------\n", self->indent, "");
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitKey(void *_self, SEMKey *sem)
{
    ABCSEMDumper *self = _self;
    dump(self, sem, INTEGER(sem, transpose), INTEGER(sem, octave), NULL);
    NoteTableDump(sem->noteTable, self->indent + 4);
}

static void visitMeter(void *self, SEMMeter *sem)
{
    dump(self, sem, INTEGER(sem, numerator), INTEGER(sem, denominator), NULL);
}

static void visitUnitNoteLength(void *self, SEMUnitNoteLength *sem)
{
    dump(self, sem, INTEGER(sem, length), NULL);
}

static void visitTempo(void *self, SEMTempo *sem)
{
    dump(self, sem, FLOAT(sem, tempo), NULL);
}

static void visitPart(void *_self, SEMPart *sem)
{
    ABCSEMDumper *self = _self;

    dump(self, sem, STRING(sem, identifier), NULL);
    self->indent += 4;

    NAIterator *iterator;
   
    iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    iterator = NAMapGetIterator(sem->voiceMap);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        Node *node = entry->value;
        printf("%*s---------- voice ----------\n", self->indent, "");
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitVoice(void *_self, SEMVoice *sem)
{
    ABCSEMDumper *self = _self;

    dump(self, sem, STRING(sem, identifier), INTEGER(sem, transpose), INTEGER(sem, octave), NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitNote(void *_self, SEMNote *sem)
{
    ABCSEMDumper *self = _self;
    Node *node = (Node *)sem;

    printf("%*s[%s]", self->indent, "", node->type);

    printf(" baseNote=%s", BaseNote2String(sem->baseNote));
    printf(" accidental=%s", Accidental2String(sem->accidental));
    printf(" %s=%s", INTEGER(sem, octave));
    printf(" %s=%s", INTEGER(sem, length.multiplier));
    printf(" %s=%s", INTEGER(sem, length.divider));

    printf(" from %s:%d:%d\n", node->location.filepath, node->location.line, node->location.column);
}

static void visitBrokenRhythm(void *self, SEMBrokenRhythm *sem)
{
    dump(self, sem, CHAR(sem, direction), NULL);
}

static void visitRest(void *self, SEMRest *sem)
{
    dump(self, sem, "type", SEMRestType2String(sem->type), INTEGER(sem, length.multiplier), INTEGER(sem, length.divider), NULL);
}

static void visitRepeat(void *self, SEMRepeat *sem)
{
    dump(self, sem, "type", SEMRepeatType2String(sem->type), INTEGER(sem, nth), NULL);
}

static void visitBarLine(void *self, SEMBarLine *sem)
{
    dump(self, sem, NULL);
}

static void visitTie(void *self, SEMTie *sem)
{
    dump(self, sem, NULL);
}

static void visitGraceNote(void *_self, SEMGraceNote *sem)
{
    ABCSEMDumper *self = _self;

    dump(self, sem, NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitTuplet(void *self, SEMTuplet *sem)
{
    dump(self, sem, INTEGER(sem, p), INTEGER(sem, q), INTEGER(sem, r), NULL);
}

static void visitChord(void *_self, SEMChord *sem)
{
    ABCSEMDumper *self = _self;

    dump(self, sem, INTEGER(sem, length.multiplier), INTEGER(sem, length.divider), NULL);
    self->indent += 4;

    NAIterator *iterator = NAArrayGetIterator(sem->node.children);
    while (iterator->hasNext(iterator)) {
        Node *node = iterator->next(iterator);
        node->accept(node, self);
    }

    self->indent -= 4;
}

static void visitOverlay(void *self, SEMOverlay *sem)
{
    dump(self, sem, NULL);
}

static void visitMidiVoice(void *self, SEMMidiVoice *sem)
{
    dump(self, sem, INTEGER(sem, instrument), INTEGER(sem, bank), NULL);
}

static void visitPropagateAccidental(void *self, SEMPropagateAccidental *sem)
{
    dump(self, sem, BOOL(sem, untilBar), BOOL(sem, allOctave), NULL);
}


Analyzer *ABCSEMDumperCreate(ParseContext *context)
{
    ABCSEMDumper *self = calloc(1, sizeof(ABCSEMDumper));
    
    self->visitor.visitFile = visitFile;
    self->visitor.visitTune = visitTune;
    self->visitor.visitKey = visitKey;
    self->visitor.visitMeter = visitMeter;
    self->visitor.visitUnitNoteLength = visitUnitNoteLength;
    self->visitor.visitTempo = visitTempo;
    self->visitor.visitPart = visitPart;
    self->visitor.visitVoice = visitVoice;
    self->visitor.visitNote = visitNote;
    self->visitor.visitBrokenRhythm = visitBrokenRhythm;
    self->visitor.visitRest = visitRest;
    self->visitor.visitRepeat = visitRepeat;
    self->visitor.visitBarLine = visitBarLine;
    self->visitor.visitTie = visitTie;
    self->visitor.visitGraceNote = visitGraceNote;
    self->visitor.visitTuplet = visitTuplet;
    self->visitor.visitChord = visitChord;
    self->visitor.visitOverlay = visitOverlay;
    self->visitor.visitMidiVoice = visitMidiVoice;
    self->visitor.visitPropagateAccidental = visitPropagateAccidental;

    self->analyzer.process = process;
    self->analyzer.destroy = destroy;
    self->analyzer.self = self;

    return &self->analyzer;
}
