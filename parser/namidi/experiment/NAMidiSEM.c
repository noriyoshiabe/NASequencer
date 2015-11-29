#include "NAMidiSEM.h"

#include <stdlib.h>

#define DeclareAccept(type) \
    static void type##Accept(void *self, void *visitor) \
    { \
        ((SEMVisitor *)visitor)->visit##type(visitor, self); \
    }

#define DeclareDestroy(type, yield) \
    static void type##Destroy(void *_self) \
    { \
        SEM##type *self = _self; \
        yield; \
        free(self); \
    }

#define NOP do { } while (0)

DeclareAccept(List);
DeclareAccept(Resolution);
DeclareAccept(Title);
DeclareAccept(Tempo);
DeclareAccept(Time);
DeclareAccept(Key);
DeclareAccept(Marker);
DeclareAccept(Channel);
DeclareAccept(Voice);
DeclareAccept(Synth);
DeclareAccept(Volume);
DeclareAccept(Pan);
DeclareAccept(Chorus);
DeclareAccept(Reverb);
DeclareAccept(Transpose);
DeclareAccept(Rest);
DeclareAccept(Note);
DeclareAccept(Pattern);
DeclareAccept(Context);

DeclareDestroy(List, { NAMapTraverseValue(self->patternMap, NodeDestroy); NAMapDestroy(self->patternMap); });
DeclareDestroy(Resolution, NOP);
DeclareDestroy(Title, free(self->title));
DeclareDestroy(Tempo, NOP);
DeclareDestroy(Time, NOP);
DeclareDestroy(Key, NoteTableRelease(self->noteTable));
DeclareDestroy(Marker, free(self->text));
DeclareDestroy(Channel, NOP);
DeclareDestroy(Voice, NOP);
DeclareDestroy(Synth, free(self->name));
DeclareDestroy(Volume, NOP);
DeclareDestroy(Pan, NOP);
DeclareDestroy(Chorus, NOP);
DeclareDestroy(Reverb, NOP);
DeclareDestroy(Transpose, NOP);
DeclareDestroy(Rest, NOP);
DeclareDestroy(Note, free(self->noteString));
DeclareDestroy(Pattern, { free(self->identifier); NAArrayTraverse(self->ctxIdList, free); NAArrayDestroy(self->ctxIdList); });
DeclareDestroy(Context, { NAArrayTraverse(self->ctxIdList, free); NAArrayDestroy(self->ctxIdList); });

void *NAMidiSEMNodeCreate(SEMType type, const char *filepath, int line, int column)
{
#define CASE(type) \
    case type: \
        return NodeCreate(sizeof(SEM##type), #type, filepath, line, column, type##Accept, type##Destroy);

    switch (type) {
    CASE(List);
    CASE(Resolution);
    CASE(Title);
    CASE(Tempo);
    CASE(Time);
    CASE(Key);
    CASE(Marker);
    CASE(Channel);
    CASE(Voice);
    CASE(Synth);
    CASE(Volume);
    CASE(Pan);
    CASE(Chorus);
    CASE(Reverb);
    CASE(Transpose);
    CASE(Rest);
    CASE(Note);
    CASE(Pattern);
    CASE(Context);
    }
#undef CASE
}
