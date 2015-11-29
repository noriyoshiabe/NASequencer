#include "NAMidiAST.h"

#include <stdlib.h>

#define DeclareAccept(type) \
    static void type##Accept(void *self, void *visitor) \
    { \
        ((ASTVisitor *)visitor)->visit##type(visitor, self); \
    }

#define DeclareDestroy(type, yield) \
    static void type##Destroy(void *_self) \
    { \
        AST##type *self = _self; \
        yield; \
        free(self); \
    }

#define NOP do { } while (0)

DeclareAccept(Root);
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
DeclareAccept(Include);
DeclareAccept(Pattern);
DeclareAccept(Define);
DeclareAccept(Context);
DeclareAccept(Identifier);
DeclareAccept(NoteParam);

DeclareDestroy(Root, NOP);
DeclareDestroy(Resolution, NOP);
DeclareDestroy(Title, free(self->title));
DeclareDestroy(Tempo, NOP);
DeclareDestroy(Time, NOP);
DeclareDestroy(Key, free(self->keyString));
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
DeclareDestroy(Note, { free(self->noteString); });
DeclareDestroy(Include, free(self->filepath));
DeclareDestroy(Pattern, { free(self->identifier); });
DeclareDestroy(Define, { free(self->identifier); });
DeclareDestroy(Context, { NAArrayTraverse(self->ctxIdList, NodeDestroy); NAArrayDestroy(self->ctxIdList); });
DeclareDestroy(Identifier, free(self->idString));
DeclareDestroy(NoteParam, NOP);

void *NAMidiASTNodeCreate(ASTType type, const char *filepath, int line, int column)
{
#define CASE(type) \
    case type: \
        return NodeCreate(sizeof(AST##type), #type, filepath, line, column, type##Accept, type##Destroy);

    switch (type) {
    CASE(Root);
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
    CASE(Include);
    CASE(Pattern);
    CASE(Define);
    CASE(Context);
    CASE(Identifier);
    CASE(NoteParam);
    }
#undef CASE
}
