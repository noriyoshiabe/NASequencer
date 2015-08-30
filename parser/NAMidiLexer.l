%{
  
#include "NAMidiParser.h"
#include <string.h>

#define YY_USER_ACTION \
  if (strchr(yytext, '\n')) ++yylineno, yycolumn = 0; \
  else yycolumn += yyleng;

%}

%option prefix="NAMidi_"
%option outfile="NAMidiLexer.c"
%option header-file="NAMidiLexer.h"
%option reentrant
%option noyywrap
%option bison-bridge
%option case-insensitive

%x COMMENT

NONZERO        [1-9]
DIGIT          [[:digit:]]
INTEGER        {NONZERO}{DIGIT}*|0
FLOAT          ({NONZERO}{DIGIT}*|0)\.{DIGIT}+
STRING         \"[^\"]*\"|'[^']*'
WSPACE         [[:blank:]]+

RESOLUTION     resolution
TITLE          title
TEMPO          tempo
TIME           time
MARKER         marker
DEFINE         define
END            end
TRACK          track
CHANNEL        channel
VOICE          voice
VOLUME         volume
PAN            pan
CHORUS         chorus
REVERB         reverb
TRANSPOSE      transpose
KEY            key

PLUS           "+"
MINUS          "-"
DIVISION       "/"
COLON          ":"
SEMICOLON      ";"

INCLUDE        include

KEY_SIGN       [cdefgab](#|b)?(maj|min)
NOTE           [cdefgab](#|b|n)?\-?[[:digit:]]{1,2}
IDENTIFIER     [_[:alpha:]][_\-[:alnum:]]+

EOL            \n

LINE_COMMENT   (\/\/|==|--).*$

%%

<<EOF>>          { return 0; }

{INTEGER}        { yylval->i = atoi(yytext); return INTEGER; }
{FLOAT}          { yylval->f = atof(yytext); return FLOAT; }
{STRING}         { yylval->s = strdup(yytext + 1);
                   yylval->s[strlen(yylval->s) - 1] = '\0';
                   return STRING; }

{WSPACE}         { }

{RESOLUTION}     { return RESOLUTION; }
{TITLE}          { return TITLE; }
{TEMPO}          { return TEMPO; }
{TIME}           { return TIME; }
{MARKER}         { return MARKER; }
{DEFINE}         { return DEFINE; }
{END}            { return END; }
{TRACK}          { return TRACK; }
{CHANNEL}        { return CHANNEL; }
{VOICE}          { return VOICE; }
{VOLUME}         { return VOLUME; }
{PAN}            { return PAN; }
{CHORUS}         { return CHORUS; }
{REVERB}         { return REVERB; }
{TRANSPOSE}      { return TRANSPOSE; }
{KEY}            { return KEY; }

{PLUS}           { return PLUS; }
{MINUS}          { return MINUS; }
{DIVISION}       { return DIVISION; }
{COLON}          { return COLON; }

{INCLUDE}        { return INCLUDE; }

{NOTE}           { yylval->s = strdup(yytext); return NOTE; }
{KEY_SIGN}       { yylval->s = strdup(yytext); return KEY_SIGN; }
{IDENTIFIER}     { yylval->s = strdup(yytext); return IDENTIFIER; }

{EOL}            { return EOL; }
{SEMICOLON}      { return SEMICOLON; }

{LINE_COMMENT}   { }
"/*"             { BEGIN(COMMENT); }
<COMMENT>"*/"    { BEGIN(INITIAL); }
<COMMENT>\n      { }
<COMMENT>.       { }

.                { return 1; }
 
%%
