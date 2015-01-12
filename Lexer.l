%{
  
#include "Parser.h"
 
#include <stdio.h>
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
NOTE_NO        {KEY}[#b]?({DIGIT}|[+-]{DIGIT})?
LOCATION       {NONZERO}{DIGIT}{0,2}:{NONZERO}{DIGIT}?(:{DIGIT}{1,3})?
MEASURE        {NONZERO}{DIGIT}*m
BEAT           {NONZERO}{DIGIT}*b

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
STEP           step
CUTOFF         cutoff
OFFSET         offset
LENGTH         length

REST           "^"
TIE            "~"
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

{INTEGER}        { return INTEGER; }
{FLOAT}          { return FLOAT; }
{STRING}         { return STRING; }
{WSPACE}         { }

{NOTE_NO}        { return NOTE_NO; }
{LOCATION}       { return LOCATION; }
{MEASURE}        { return MEASURE; }
{BEAT}           { return BEAT; }

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
{STEP}           { return STEP; }
{CUTOFF}         { return CUTOFF; }
{OFFSET}         { return OFFSET; }
{LENGTH}         { return LENGTH; }

{REST}           { return REST; }
{TIE}            { return TIE; }
{ASSIGN}         { return ASSIGN; }
{SEMICOLON}      { return SEMICOLON; }
{COMMA}          { return COMMA; }

{LPAREN}         { return LPAREN; }
{RPAREN}         { return RPAREN; }
{LCURLY}         { return LCURLY; }
{RCURLY}         { return RCURLY; }

{IDENTIFIER}     { return IDENTIFIER; }

{EOL}            { return EOL; }

{LINE_COMMENT}   { }
"/*"             { BEGIN(COMMENT); }
<COMMENT>"*/"    { BEGIN(INITIAL); }
<COMMENT>\n      { }
<COMMENT>.       { }
 
%%
