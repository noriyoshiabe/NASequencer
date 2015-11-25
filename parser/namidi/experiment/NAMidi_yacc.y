%{
 
#include "NAMidiParser.h"
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"

#include <stdio.h>
#include <stdlib.h>

extern int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, const char *message);
extern void NAMidi_lex_set_error_until_eol(yyscan_t scanner);

#define __Trace__ printf("%d: -- %s\n", __LINE__, NAMidi_get_text(scanner));

#define stmt(type) (StatementCreate(sizeof(type), #type, type##Accept, type##Destroy))
#define list() NodeListCreate()
#define listAppend(list, node) NodeListAppend(list, node)
#define listDestroy(list) do { NodeListTraverse(list, StatementDestroy); free(list); } while (0)

#define DeclareStatementPrototype(type) \
    static void type##Accept(void *self, void *visitor); \
    static void type##Destroy(void *self);

DeclareStatementPrototype(Resolution);
DeclareStatementPrototype(Title);
DeclareStatementPrototype(Tempo);
DeclareStatementPrototype(Time);
DeclareStatementPrototype(Key);
DeclareStatementPrototype(Marker);
DeclareStatementPrototype(Channel);
DeclareStatementPrototype(Voice);
DeclareStatementPrototype(Synth);
DeclareStatementPrototype(Volume);
DeclareStatementPrototype(Pan);
DeclareStatementPrototype(Chorus);
DeclareStatementPrototype(Reverb);
DeclareStatementPrototype(Transpose);
DeclareStatementPrototype(Rest);
DeclareStatementPrototype(Note);
DeclareStatementPrototype(Include);
DeclareStatementPrototype(Pattern);
DeclareStatementPrototype(Define);
DeclareStatementPrototype(Context);
DeclareStatementPrototype(Identifier);
DeclareStatementPrototype(NoteParam);

%}

%name-prefix = "NAMidi_"
%output = "NAMidi_yacc.c"
%defines

%pure-parser
%lex-param   { yyscan_t scanner }
%parse-param { yyscan_t scanner }
%parse-param { const char *filepath }
%locations

%union {
    int i;
    float f;
    char *s;
    Node *node;
    NodeList *list;
}

%token <i>INTEGER
%token <f>FLOAT
%token <s>STRING

%token RESOLUTION TITLE TEMPO TIME KEY MARKER DEFINE END CHANNEL VOICE
       SYNTH VOLUME PAN CHORUS REVERB TRANSPOSE DEFAULT REST INCLUDE

%token <s>NOTE KEY_SIGN IDENTIFIER

%type <node> resolution title tempo time key marker channel voice synth volume
             pan chorus reverb transpose rest note include pattern

%type <node> define context

%type <node> statement note_param identifier
%type <list> statement_list identifier_list note_param_list

%%

input
    : statement_list
    ;

statement_list
    :
        {
            $$ = list();
        }
    | statement
        {
            $$ = $1 ? listAppend(list(), $1) : list();
        }
    | statement_list statement
        {
            $$ = $2 ? listAppend($1, $2) : $1;
        }
    ;

statement
    : resolution
    | title
    | tempo
    | time
    | key
    | marker
    | channel
    | voice
    | synth
    | volume
    | pan
    | chorus
    | reverb
    | transpose
    | rest
    | note
    | include
    | pattern
    | define
    | context

    | ';'
        {
            $$ = NULL;
        }
    | error
        {
            NAMidi_lex_set_error_until_eol(scanner);
            $$ = NULL;
        }
    ;

resolution
    : RESOLUTION INTEGER
        {
            Resolution *n = stmt(Resolution);
            n->resolution = $2;
            $$ = (Node *)n;
        }
    ;

title
    : TITLE STRING
        {
            Title *n = stmt(Title);
            n->title = $2;
            $$ = (Node *)n;
        }
    ;

tempo
    : TEMPO FLOAT
        {
            Tempo *n = stmt(Tempo);
            n->tempo = $2;
            $$ = (Node *)n;
        }
    ;
      
time
    : TIME INTEGER '/' INTEGER
        {
            Time *n = stmt(Time);
            n->numerator = $2;
            n->denominator = $4;
            $$ = (Node *)n;
        }
    ;
      
key
    : KEY KEY_SIGN
        {
            Key *n = stmt(Key);
            n->keyString = $2;
            $$ = (Node *)n;
        }
    ;

marker
    : MARKER STRING
        {
            Marker *n = stmt(Marker);
            n->text = $2;
            $$ = (Node *)n;
        }
    ;

channel
    : CHANNEL INTEGER
        {
            Channel *n = stmt(Channel);
            n->number = $2;
            $$ = (Node *)n;
        }
    ;

voice
    : VOICE INTEGER INTEGER INTEGER
        {
            Voice *n = stmt(Voice);
            n->msb = $2;
            n->lsb = $3;
            n->programNo = $4;
            $$ = (Node *)n;
        }
    ;

synth
    : SYNTH STRING
        {
            Synth *n = stmt(Synth);
            n->name = $2;
            $$ = (Node *)n;
        }
    ;

volume
    : VOLUME INTEGER
        {
            Volume *n = stmt(Volume);
            n->value = $2;
            $$ = (Node *)n;
        }
    ;

pan
    : PAN INTEGER
        {
            Pan *n = stmt(Pan);
            n->value = $2;
            $$ = (Node *)n;
        }
    | PAN '+' INTEGER
        {
            Pan *n = stmt(Pan);
            n->value = $3;
            $$ = (Node *)n;
        }
    | PAN '-' INTEGER
        {
            Pan *n = stmt(Pan);
            n->value = -$3;
            $$ = (Node *)n;
        }
    ;

chorus
    : CHORUS INTEGER
        {
            Chorus *n = stmt(Chorus);
            n->value = $2;
            $$ = (Node *)n;
        }
    ;

reverb
    : REVERB INTEGER
        {
            Reverb *n = stmt(Reverb);
            n->value = $2;
            $$ = (Node *)n;
        }
    ;

transpose
    : TRANSPOSE INTEGER
        {
            Transpose *n = stmt(Transpose);
            n->value = $2;
            $$ = (Node *)n;
        }
    | TRANSPOSE '+' INTEGER
        {
            Transpose *n = stmt(Transpose);
            n->value = $3;
            $$ = (Node *)n;
        }
    | TRANSPOSE '-' INTEGER
        {
            Transpose *n = stmt(Transpose);
            n->value = -$3;
            $$ = (Node *)n;
        }
    ;

rest
    : REST INTEGER
        {
            Rest *n = stmt(Rest);
            n->step = $2;
            $$ = (Node *)n;
        }
    ;

note
    : NOTE note_param_list
        {
            Note *n = stmt(Note);
            n->noteString = $1;
            n->list = $2;
            $$ = (Node *)n;
        }
    ;

include
    : INCLUDE STRING
        {
            Include *n = stmt(Include);
            n->filepath = $2;
            $$ = (Node *)n;
        }
    ;

pattern
    : IDENTIFIER
        {
            Pattern *n = stmt(Pattern);
            n->identifier = $1;
            n->list = list();
            $$ = (Node *)n;
        }
    | IDENTIFIER '(' identifier_list ')'
        {
            Pattern *n = stmt(Pattern);
            n->identifier = $1;
            n->list = $3;
            $$ = (Node *)n;
        }
    ;

define
    : DEFINE IDENTIFIER statement_list END
        {
            Define *n = stmt(Define);
            n->identifier = $2;
            n->list = $3;
            $$ = (Node *)n;
        }
    ;

context
    : identifier_list '{' statement_list '}'
        {
            Context *n = stmt(Context);
            n->ctxIdList = $1;
            n->list = $3;
            $$ = (Node *)n;
        }
    ;

identifier_list
    :
        {
            $$ = list();
        }
    | identifier
        {
            $$ = listAppend(list(), $1);
        }
    | identifier_list ',' identifier
        {
            $$ = listAppend($1, $3);
        }
    ;

identifier
    : IDENTIFIER
        {
            Identifier *n = stmt(Identifier);
            n->idString = $1;
            $$ = (Node *)n;
        }
    ;

note_param_list
    :
        {
            $$ = list();
        }
    | note_param
        {
            $$ = listAppend(list(), $1);
        }
    | note_param_list note_param
        {
            $$ = listAppend($1, $2);
        }
    ;

note_param
    : '-'
        {
            NoteParam *n = stmt(NoteParam);
            n->value = -1;
            $$ = (Node *)n;
        }
    | INTEGER
        {
            NoteParam *n = stmt(NoteParam);
            n->value = $1;
            $$ = (Node *)n;
        }
    ;

%%

#define DeclareAccept(type) \
    static void type##Accept(void *self, void *visitor) \
    { \
        NAMidiParserVisit##type(visitor, self); \
    }

#define DeclareDestroy(type, yield) \
    static void type##Destroy(void *_self) \
    { \
        type *self = _self; \
        yield; \
        free(self); \
    }

#define NOP do { } while (0)

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
DeclareDestroy(Note, { free(self->noteString); listDestroy(self->list); });
DeclareDestroy(Include, free(self->filepath));
DeclareDestroy(Pattern, { free(self->identifier); listDestroy(self->list); });
DeclareDestroy(Define, { free(self->identifier); listDestroy(self->list); });
DeclareDestroy(Context, { listDestroy(self->ctxIdList); listDestroy(self->list); });
DeclareDestroy(Identifier, free(self->idString));
DeclareDestroy(NoteParam, NOP);
