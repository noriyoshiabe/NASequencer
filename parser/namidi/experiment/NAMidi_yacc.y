%{
 
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"

#include <ctype.h>

extern int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, const char *message);
extern void NAMidi_lex_set_error(yyscan_t scanner);

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
}

%token <i>INTEGER
%token <f>FLOAT
%token <s>STRING

%token RESOLUTION
%token TITLE
%token TEMPO
%token TIME
%token MARKER
%token DEFINE
%token END
%token CHANNEL
%token VOICE
%token SYNTH
%token VOLUME
%token PAN
%token CHORUS
%token REVERB
%token TRANSPOSE
%token KEY
%token DEFAULT
%token REST
%token INCLUDE

%token <s>NOTE
%token <s>KEY_SIGN
%token <s>IDENTIFIER

%%
 
input
    :
    | tokens 
    ;

tokens
    : token {
                 char *text = NAMidi_get_text(scanner);
                 printf("-- %s\n", text[0] == '\n' ? "eol" : text);
            }
    | tokens token {
                 char *text = NAMidi_get_text(scanner);
                 printf("-- %s\n", text[0] == '\n' ? "eol" : text);
            }
    | error { NAMidi_lex_set_error(scanner); }
    ;

token
    : INTEGER
    | FLOAT
    | STRING
   
    | RESOLUTION
    | TITLE
    | TEMPO
    | TIME
    | MARKER
    | DEFINE
    | END
    | CHANNEL
    | VOICE
    | SYNTH
    | VOLUME
    | PAN
    | CHORUS
    | REVERB
    | TRANSPOSE
    | KEY
    | DEFAULT
    | REST
    | INCLUDE

    | NOTE
    | KEY_SIGN
    | IDENTIFIER
    
    | '+'
    | '-'
    | '/'
    | '('
    | ')'
    | '{'
    | '}'
    | ','
    | ';'
    
    
    ;

%%
