%{
 
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"

#include <ctype.h>

extern int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, const char *message);
extern void NAMidi_lex_set_error_until_eol(yyscan_t scanner);

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

%type <node> statement pattern_statement context_statement
             note_param
%type <list> statement_list pattern_statement_list context_statement_list
             identifier_list note_param_list

%%
 
statement_list
    :                          { $$ = NULL; }
    | statement                { $$ = NULL; }
    | statement_list statement { $$ = NULL; }
    | error                    { $$ = NULL; NAMidi_lex_set_error_until_eol(scanner); }
    ;

pattern_statement_list
    : pattern_statement                        { $$ = NULL; }
    | pattern_statement_list pattern_statement { $$ = NULL; }
    | error                                    { $$ = NULL; NAMidi_lex_set_error_until_eol(scanner); }
    ;

context_statement_list
    : context_statement                        { $$ = NULL; }
    | context_statement_list context_statement { $$ = NULL; }
    | error                                    { $$ = NULL; NAMidi_lex_set_error_until_eol(scanner); }
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
    ;

pattern_statement
    : tempo
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
    | pattern
    | define
    | context
    ;

context_statement
    : tempo
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
    ;

resolution
    : RESOLUTION INTEGER { $$ = NULL; }
    ;

title
    : TITLE STRING { $$ = NULL; }
    ;

tempo
    : TEMPO FLOAT { $$ = NULL; }
    ;
      
time
    : TIME INTEGER '/' INTEGER { $$ = NULL; }
    ;
      
key
    : KEY KEY_SIGN { $$ = NULL; }
    ;

marker
    : MARKER STRING { $$ = NULL; }
    ;

channel
    : CHANNEL INTEGER { $$ = NULL; }
    ;

voice
    : VOICE INTEGER INTEGER INTEGER { $$ = NULL; }
    ;

synth
    : SYNTH STRING { $$ = NULL; }
    ;

volume
    : VOLUME INTEGER { $$ = NULL; }
    ;

pan
    : PAN INTEGER     { $$ = NULL; }
    | PAN '+' INTEGER { $$ = NULL; }
    | PAN '-' INTEGER { $$ = NULL; }
    ;

chorus
    : CHORUS INTEGER { $$ = NULL; }
    ;

reverb
    : REVERB INTEGER { $$ = NULL; }
    ;

transpose
    : TRANSPOSE INTEGER     { $$ = NULL; }
    | TRANSPOSE '+' INTEGER { $$ = NULL; }
    | TRANSPOSE '-' INTEGER { $$ = NULL; }
    ;

rest
    : REST INTEGER { $$ = NULL; }
    ;

note
    : NOTE note_param_list { $$ = NULL; } 
    ;

include
    : INCLUDE STRING { $$ = NULL; }
    ;

pattern
    : IDENTIFIER                         { $$ = NULL; }
    | IDENTIFIER '(' identifier_list ')' { $$ = NULL; }
    ;

define
    : DEFINE IDENTIFIER pattern_statement_list END { $$ = NULL; }
    ;

context
    : identifier_list '{' context_statement_list '}' { $$ = NULL; }
    ;

identifier_list
    :                                { $$ = NULL; }
    | IDENTIFIER                     { $$ = NULL; } 
    | identifier_list ',' IDENTIFIER { $$ = NULL; }
    ;

note_param_list
    :                            { $$ = NULL; }
    | note_param                 { $$ = NULL; }
    | note_param_list note_param { $$ = NULL; }
    ;

note_param
    : '-'     { $$ = NULL; }
    | INTEGER { $$ = NULL; }
    ;

%%
