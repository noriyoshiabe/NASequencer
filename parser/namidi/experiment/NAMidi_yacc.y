%{
 
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"

#include <ctype.h>

extern int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, const char *message);
extern void NAMidi_lex_set_error_until_eol(yyscan_t scanner);

#define __Trace__ printf("%d: -- %s\n", __LINE__, NAMidi_get_text(scanner));

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

%type <node> statement note_param
%type <list> statement_list identifier_list note_param_list

%%

input
    : statement_list { __Trace__ }
    ;

statement_list
    :                          { __Trace__ $$ = NULL; }
    | statement                { __Trace__ $$ = NULL; }
    | statement_list statement { __Trace__ $$ = NULL; }
    ;

statement
    : resolution { __Trace__ }
    | title      { __Trace__ }
    | tempo      { __Trace__ }
    | time       { __Trace__ }
    | key        { __Trace__ }
    | marker     { __Trace__ }
    | channel    { __Trace__ }
    | voice      { __Trace__ }
    | synth      { __Trace__ }
    | volume     { __Trace__ }
    | pan        { __Trace__ }
    | chorus     { __Trace__ }
    | reverb     { __Trace__ }
    | transpose  { __Trace__ }
    | rest       { __Trace__ }
    | note       { __Trace__ }
    | include    { __Trace__ }
    | pattern    { __Trace__ }
    | define     { __Trace__ }
    | context    { __Trace__ }

    | ';'        { __Trace__ $$ = NULL; }
    | error      { NAMidi_lex_set_error_until_eol(scanner); }
    ;

resolution
    : RESOLUTION INTEGER { __Trace__ $$ = NULL; }
    ;

title
    : TITLE STRING { __Trace__ $$ = NULL; }
    ;

tempo
    : TEMPO FLOAT { __Trace__ $$ = NULL; }
    ;
      
time
    : TIME INTEGER '/' INTEGER { __Trace__ $$ = NULL; }
    ;
      
key
    : KEY KEY_SIGN { __Trace__ $$ = NULL; }
    ;

marker
    : MARKER STRING { __Trace__ $$ = NULL; }
    ;

channel
    : CHANNEL INTEGER { __Trace__ $$ = NULL; }
    ;

voice
    : VOICE INTEGER INTEGER INTEGER { __Trace__ $$ = NULL; }
    ;

synth
    : SYNTH STRING { __Trace__ $$ = NULL; }
    ;

volume
    : VOLUME INTEGER { __Trace__ $$ = NULL; }
    ;

pan
    : PAN INTEGER     { __Trace__ $$ = NULL; }
    | PAN '+' INTEGER { __Trace__ $$ = NULL; }
    | PAN '-' INTEGER { __Trace__ $$ = NULL; }
    ;

chorus
    : CHORUS INTEGER { __Trace__ $$ = NULL; }
    ;

reverb
    : REVERB INTEGER { __Trace__ $$ = NULL; }
    ;

transpose
    : TRANSPOSE INTEGER     { __Trace__ $$ = NULL; }
    | TRANSPOSE '+' INTEGER { __Trace__ $$ = NULL; }
    | TRANSPOSE '-' INTEGER { __Trace__ $$ = NULL; }
    ;

rest
    : REST INTEGER { __Trace__ $$ = NULL; }
    ;

note
    : NOTE note_param_list { __Trace__ $$ = NULL; } 
    ;

include
    : INCLUDE STRING { __Trace__ $$ = NULL; }
    ;

pattern
    : IDENTIFIER                         { __Trace__ $$ = NULL; }
    | IDENTIFIER '(' identifier_list ')' { __Trace__ $$ = NULL; }
    ;

define
    : DEFINE IDENTIFIER statement_list END { __Trace__ $$ = NULL; }
    ;

context
    : identifier_list '{' statement_list '}' { __Trace__ $$ = NULL; }
    ;

identifier_list
    :                                { __Trace__ $$ = NULL; }
    | IDENTIFIER                     { __Trace__ $$ = NULL; } 
    | identifier_list ',' IDENTIFIER { __Trace__ $$ = NULL; }
    ;

note_param_list
    :                            { __Trace__ $$ = NULL; }
    | note_param                 { __Trace__ $$ = NULL; }
    | note_param_list note_param { __Trace__ $$ = NULL; }
    ;

note_param
    : '-'     { __Trace__ $$ = NULL; }
    | INTEGER { __Trace__ $$ = NULL; }
    ;

%%
