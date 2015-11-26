%{
 
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"

#include "NAMidiStatement.h"
#include "NAMidiStatementVisitor.h"

#include <stdio.h>
#include <stdlib.h>

extern int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, const char *message);
extern void NAMidi_lex_set_error_until_eol(yyscan_t scanner);

#define node(type) (NodeCreate(sizeof(type), #type, type##Accept, type##Destroy))
#define list() NAArrayCreate(4, NULL)
#define listAppend(list, node) NAArrayAppend(list, node)

static void ResolutionAccept(void *self, void *visitor);
static void TitleAccept(void *self, void *visitor);
static void TempoAccept(void *self, void *visitor);
static void TimeAccept(void *self, void *visitor);
static void KeyAccept(void *self, void *visitor);
static void MarkerAccept(void *self, void *visitor);
static void ChannelAccept(void *self, void *visitor);
static void VoiceAccept(void *self, void *visitor);
static void SynthAccept(void *self, void *visitor);
static void VolumeAccept(void *self, void *visitor);
static void PanAccept(void *self, void *visitor);
static void ChorusAccept(void *self, void *visitor);
static void ReverbAccept(void *self, void *visitor);
static void TransposeAccept(void *self, void *visitor);
static void RestAccept(void *self, void *visitor);
static void NoteAccept(void *self, void *visitor);
static void IncludeAccept(void *self, void *visitor);
static void PatternAccept(void *self, void *visitor);
static void DefineAccept(void *self, void *visitor);
static void ContextAccept(void *self, void *visitor);
static void IdentifierAccept(void *self, void *visitor);
static void NoteParamAccept(void *self, void *visitor);

static void ResolutionDestroy(void *self);
static void TitleDestroy(void *self);
static void TempoDestroy(void *self);
static void TimeDestroy(void *self);
static void KeyDestroy(void *self);
static void MarkerDestroy(void *self);
static void ChannelDestroy(void *self);
static void VoiceDestroy(void *self);
static void SynthDestroy(void *self);
static void VolumeDestroy(void *self);
static void PanDestroy(void *self);
static void ChorusDestroy(void *self);
static void ReverbDestroy(void *self);
static void TransposeDestroy(void *self);
static void RestDestroy(void *self);
static void NoteDestroy(void *self);
static void IncludeDestroy(void *self);
static void PatternDestroy(void *self);
static void DefineDestroy(void *self);
static void ContextDestroy(void *self);
static void IdentifierDestroy(void *self);
static void NoteParamDestroy(void *self);

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
    void *node;
    void *list;
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
            $$ = list();
            if ($1) {
                listAppend($$, $1);
            }
        }
    | statement_list statement
        {
            $$ = $1;
            if ($2) {
                listAppend($$, $2);
            }
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
            Resolution *n = node(Resolution);
            n->resolution = $2;
            $$ = n;
        }
    ;

title
    : TITLE STRING
        {
            Title *n = node(Title);
            n->title = $2;
            $$ = n;
        }
    ;

tempo
    : TEMPO FLOAT
        {
            Tempo *n = node(Tempo);
            n->tempo = $2;
            $$ = n;
        }
    ;
      
time
    : TIME INTEGER '/' INTEGER
        {
            Time *n = node(Time);
            n->numerator = $2;
            n->denominator = $4;
            $$ = n;
        }
    ;
      
key
    : KEY KEY_SIGN
        {
            Key *n = node(Key);
            n->keyString = $2;
            $$ = n;
        }
    ;

marker
    : MARKER STRING
        {
            Marker *n = node(Marker);
            n->text = $2;
            $$ = n;
        }
    ;

channel
    : CHANNEL INTEGER
        {
            Channel *n = node(Channel);
            n->number = $2;
            $$ = n;
        }
    ;

voice
    : VOICE INTEGER INTEGER INTEGER
        {
            Voice *n = node(Voice);
            n->msb = $2;
            n->lsb = $3;
            n->programNo = $4;
            $$ = n;
        }
    ;

synth
    : SYNTH STRING
        {
            Synth *n = node(Synth);
            n->name = $2;
            $$ = n;
        }
    ;

volume
    : VOLUME INTEGER
        {
            Volume *n = node(Volume);
            n->value = $2;
            $$ = n;
        }
    ;

pan
    : PAN INTEGER
        {
            Pan *n = node(Pan);
            n->value = $2;
            $$ = n;
        }
    | PAN '+' INTEGER
        {
            Pan *n = node(Pan);
            n->value = $3;
            $$ = n;
        }
    | PAN '-' INTEGER
        {
            Pan *n = node(Pan);
            n->value = -$3;
            $$ = n;
        }
    ;

chorus
    : CHORUS INTEGER
        {
            Chorus *n = node(Chorus);
            n->value = $2;
            $$ = n;
        }
    ;

reverb
    : REVERB INTEGER
        {
            Reverb *n = node(Reverb);
            n->value = $2;
            $$ = n;
        }
    ;

transpose
    : TRANSPOSE INTEGER
        {
            Transpose *n = node(Transpose);
            n->value = $2;
            $$ = n;
        }
    | TRANSPOSE '+' INTEGER
        {
            Transpose *n = node(Transpose);
            n->value = $3;
            $$ = n;
        }
    | TRANSPOSE '-' INTEGER
        {
            Transpose *n = node(Transpose);
            n->value = -$3;
            $$ = n;
        }
    ;

rest
    : REST INTEGER
        {
            Rest *n = node(Rest);
            n->step = $2;
            $$ = n;
        }
    ;

note
    : NOTE note_param_list
        {
            Note *n = node(Note);
            n->noteString = $1;
            n->node.children = $2;
            $$ = n;
        }
    ;

include
    : INCLUDE STRING
        {
            Include *n = node(Include);
            n->filepath = $2;
            $$ = n;
        }
    ;

pattern
    : IDENTIFIER
        {
            Pattern *n = node(Pattern);
            n->identifier = $1;
            n->node.children = list();
            $$ = n;
        }
    | IDENTIFIER '(' identifier_list ')'
        {
            Pattern *n = node(Pattern);
            n->identifier = $1;
            n->node.children = $3;
            $$ = n;
        }
    ;

define
    : DEFINE IDENTIFIER statement_list END
        {
            Define *n = node(Define);
            n->identifier = $2;
            n->node.children = $3;
            $$ = n;
        }
    ;

context
    : identifier_list '{' statement_list '}'
        {
            Context *n = node(Context);
            n->ctxIdList = $1;
            n->node.children = $3;
            $$ = n;
        }
    ;

identifier_list
    :
        {
            $$ = list();
        }
    | identifier
        {
            $$ = list();
            listAppend($$, $1);
        }
    | identifier_list ',' identifier
        {
            $$ = $1;
            listAppend($$, $3);
        }
    ;

identifier
    : IDENTIFIER
        {
            Identifier *n = node(Identifier);
            n->idString = $1;
            $$ = n;
        }
    ;

note_param_list
    :
        {
            $$ = list();
        }
    | note_param
        {
            $$ = list();
            listAppend($$, $1);
        }
    | note_param_list note_param
        {
            $$ = $1;
            listAppend($$, $2);
        }
    ;

note_param
    : '-'
        {
            NoteParam *n = node(NoteParam);
            n->value = -1;
            $$ = n;
        }
    | INTEGER
        {
            NoteParam *n = node(NoteParam);
            n->value = $1;
            $$ = n;
        }
    ;

%%

#define DeclareAccept(type) \
    static void type##Accept(void *self, void *visitor) \
    { \
        ((NAMidiStatementVisitor *)visitor)->visit##type(visitor, self); \
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
DeclareDestroy(Note, { free(self->noteString); });
DeclareDestroy(Include, free(self->filepath));
DeclareDestroy(Pattern, { free(self->identifier); });
DeclareDestroy(Define, { free(self->identifier); });
DeclareDestroy(Context, { NAArrayTraverse(self->ctxIdList, NodeDestroy); NAArrayDestroy(self->ctxIdList); });
DeclareDestroy(Identifier, free(self->idString));
DeclareDestroy(NoteParam, NOP);
