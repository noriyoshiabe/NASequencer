%{
 
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"

#include "NAMidiAST.h"

extern int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, void **node, const char *message);
extern void NAMidi_lex_set_error_until_eol(yyscan_t scanner);

#define node(type, yylloc) NAMidiASTNodeCreate(ASTType##type, &((FileLocation){filepath, yylloc.first_line, yylloc.first_column}))
#define list() NAArrayCreate(4, NULL)
#define listAppend(list, node) NAArrayAppend(list, node)

%}

%name-prefix = "NAMidi_"
%output = "NAMidi_yacc.c"
%defines

%pure-parser
%lex-param   { yyscan_t scanner }
%parse-param { yyscan_t scanner }
%parse-param { const char *filepath }
%parse-param { void **node }
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
        {
            ASTRoot *n = node(Root, @$);
            n->node.children = $1;
            *node = n;
        }
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
            ASTResolution *n = node(Resolution, @$);
            n->resolution = $2;
            $$ = n;
        }
    ;

title
    : TITLE STRING
        {
            ASTTitle *n = node(Title, @$);
            n->title = $2;
            $$ = n;
        }
    ;

tempo
    : TEMPO FLOAT
        {
            ASTTempo *n = node(Tempo, @$);
            n->tempo = $2;
            $$ = n;
        }
    ;
      
time
    : TIME INTEGER '/' INTEGER
        {
            ASTTime *n = node(Time, @$);
            n->numerator = $2;
            n->denominator = $4;
            $$ = n;
        }
    ;
      
key
    : KEY KEY_SIGN
        {
            ASTKey *n = node(Key, @$);
            n->keyString = $2;
            $$ = n;
        }
    ;

marker
    : MARKER STRING
        {
            ASTMarker *n = node(Marker, @$);
            n->text = $2;
            $$ = n;
        }
    ;

channel
    : CHANNEL INTEGER
        {
            ASTChannel *n = node(Channel, @$);
            n->number = $2;
            $$ = n;
        }
    ;

voice
    : VOICE INTEGER INTEGER INTEGER
        {
            ASTVoice *n = node(Voice, @$);
            n->msb = $2;
            n->lsb = $3;
            n->programNo = $4;
            $$ = n;
        }
    ;

synth
    : SYNTH STRING
        {
            ASTSynth *n = node(Synth, @$);
            n->name = $2;
            $$ = n;
        }
    ;

volume
    : VOLUME INTEGER
        {
            ASTVolume *n = node(Volume, @$);
            n->value = $2;
            $$ = n;
        }
    ;

pan
    : PAN INTEGER
        {
            ASTPan *n = node(Pan, @$);
            n->value = $2;
            $$ = n;
        }
    | PAN '+' INTEGER
        {
            ASTPan *n = node(Pan, @$);
            n->value = $3;
            $$ = n;
        }
    | PAN '-' INTEGER
        {
            ASTPan *n = node(Pan, @$);
            n->value = -$3;
            $$ = n;
        }
    ;

chorus
    : CHORUS INTEGER
        {
            ASTChorus *n = node(Chorus, @$);
            n->value = $2;
            $$ = n;
        }
    ;

reverb
    : REVERB INTEGER
        {
            ASTReverb *n = node(Reverb, @$);
            n->value = $2;
            $$ = n;
        }
    ;

transpose
    : TRANSPOSE INTEGER
        {
            ASTTranspose *n = node(Transpose, @$);
            n->value = $2;
            $$ = n;
        }
    | TRANSPOSE '+' INTEGER
        {
            ASTTranspose *n = node(Transpose, @$);
            n->value = $3;
            $$ = n;
        }
    | TRANSPOSE '-' INTEGER
        {
            ASTTranspose *n = node(Transpose, @$);
            n->value = -$3;
            $$ = n;
        }
    ;

rest
    : REST INTEGER
        {
            ASTRest *n = node(Rest, @$);
            n->step = $2;
            $$ = n;
        }
    ;

note
    : NOTE note_param_list
        {
            ASTNote *n = node(Note, @$);
            n->noteString = $1;
            n->node.children = $2;
            $$ = n;
        }
    ;

include
    : INCLUDE STRING
        {
            ASTInclude *n = node(Include, @$);
            n->filepath = $2;
            $$ = n;
        }
    ;

pattern
    : IDENTIFIER
        {
            ASTPattern *n = node(Pattern, @$);
            n->identifier = $1;
            n->node.children = list();
            $$ = n;
        }
    | IDENTIFIER '(' identifier_list ')'
        {
            ASTPattern *n = node(Pattern, @$);
            n->identifier = $1;
            n->node.children = $3;
            $$ = n;
        }
    ;

define
    : DEFINE IDENTIFIER statement_list END
        {
            ASTDefine *n = node(Define, @$);
            n->identifier = $2;
            n->node.children = $3;
            $$ = n;
        }
    ;

context
    : identifier_list '{' statement_list '}'
        {
            ASTContext *n = node(Context, @$);
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
            ASTIdentifier *n = node(Identifier, @$);
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
            ASTNoteParam *n = node(NoteParam, @$);
            n->value = -1;
            $$ = n;
        }
    | INTEGER
        {
            ASTNoteParam *n = node(NoteParam, @$);
            n->value = $1;
            $$ = n;
        }
    ;

%%