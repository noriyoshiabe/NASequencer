%{
  
#include "Parser.h"
#include <string.h>

#define YY_USER_INIT \
    yylineno = 1; \
    yycolumn = 1;


#define YY_USER_ACTION \
    yylloc->first_line = yylloc->last_line; \
    yylloc->last_line = yylineno; \
    yylloc->first_column = yycolumn; yylloc->last_column = yycolumn + yyleng - 1; \
    if (strchr(yytext, '\n')) yycolumn = 1; \
    else yycolumn += yyleng;

%}

%option outfile="Lexer.c"
%option header-file="Lexer.h"
%option reentrant
%option noyywrap
%option bison-bridge
%option bison-locations
%option yylineno
%option case-insensitive

%x COMMENT

NONZERO        [1-9]
DIGIT          [[:digit:]]
INTEGER        {NONZERO}{DIGIT}*|0
FLOAT          ({NONZERO}{DIGIT}*\.?|0\.|\.{DIGIT}){DIGIT}*|0
STRING         \"[^\"]*\"|'[^']*'
WSPACE         [[:blank:]]+

KEY            c|d|e|f|g|a|b
NOTE_NO        {KEY}(#|b)?({DIGIT}|[+-]{DIGIT})?
LOCATION       {NONZERO}{DIGIT}{0,2}:{NONZERO}{DIGIT}?(:{DIGIT}{1,3})?|m{NONZERO}{DIGIT}*|b{NONZERO}{DIGIT}*
MB_LENGTH      {NONZERO}{DIGIT}*m|{NONZERO}{DIGIT}*b

RESOLUTION     resolution
TITLE          title
TIME           time
TEMPO          tempo
MARKER         marker
SOUND          sound
SELECT         select
CHANNEL        channel
VELOCITY       velocity
GATETIME       gatetime
CUTOFF         cutoff
NOTE           note

STEP           step
FROM           from
TO             to
REPLACE        replace
MIX            mix
OFFSET         offset
LENGTH         length

REST           "^"
TIE            "~"

PLUS           "+"
MINUS          "-"
MULTIPLY       "*"
DIVISION       "/"
ASSIGN         "="

SEMICOLON      ";"
COMMA          ","

LPAREN         "("
RPAREN         ")"
LCURLY         "{"
RCURLY         "}"

IDENTIFIER     [_[:alpha:]][[:alnum:]]*

EOL            \n

LINE_COMMENT   \/\/.*$

%%

<<EOF>>          { return 0; }

{INTEGER}        { yylval->i = atoi(yytext); return INTEGER; }
{FLOAT}          { yylval->f = atof(yytext); return FLOAT; }
{STRING}         { yylval->s = strdup(yytext); return STRING; }
{WSPACE}         { }

{NOTE_NO}        { yylval->s = strdup(yytext); return NOTE_NO; }
{LOCATION}       { yylval->s = strdup(yytext); return LOCATION; }
{MB_LENGTH}      { yylval->s = strdup(yytext); return MB_LENGTH; }

{RESOLUTION}     { return RESOLUTION; }
{TITLE}          { return TITLE; }
{TIME}           { return TIME; }
{TEMPO}          { return TEMPO; }
{MARKER}         { return MARKER; }
{SOUND}          { return SOUND; }
{SELECT}         { return SELECT; }
{CHANNEL}        { return CHANNEL; }
{VELOCITY}       { return VELOCITY; }
{GATETIME}       { return GATETIME; }
{CUTOFF}         { return CUTOFF; }
{NOTE}           { return NOTE; }

{STEP}           { return STEP; }
{FROM}           { return FROM; }
{TO}             { return TO; }
{REPLACE}        { return REPLACE; }
{MIX}            { return MIX; }
{OFFSET}         { return OFFSET; }
{LENGTH}         { return LENGTH; }

{REST}           { return REST; }
{TIE}            { return TIE; }

{PLUS}           { return PLUS; }
{MINUS}          { return MINUS; }
{MULTIPLY}       { return MULTIPLY; }
{DIVISION}       { return DIVISION; }
{ASSIGN}         { return ASSIGN; }
{SEMICOLON}      { }
{COMMA}          { return COMMA; }

{LPAREN}         { return LPAREN; }
{RPAREN}         { return RPAREN; }
{LCURLY}         { return LCURLY; }
{RCURLY}         { return RCURLY; }

{IDENTIFIER}     { yylval->s = strdup(yytext); return IDENTIFIER; }

{EOL}            { }

{LINE_COMMENT}   { }
"/*"             { BEGIN(COMMENT); }
<COMMENT>"*/"    { BEGIN(INITIAL); }
<COMMENT>\n      { }
<COMMENT>.       { }

.                { return 1; }
 
%%