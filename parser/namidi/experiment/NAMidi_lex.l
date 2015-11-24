%{
  
#include "NAMidi_yacc.h"
#include <string.h>

#define YY_USER_INIT \
    yylineno = 1; \
    yycolumn = 1;

#define YY_USER_ACTION \
    yylloc->first_line = yylineno; \
    yylloc->first_column = yycolumn; \
    if (strchr(yytext, '\n')) ++yylineno, yycolumn = 1; \
    else yycolumn += yyleng;

%}

%option prefix="NAMidi_"
%option outfile="NAMidi_lex.c"
%option header-file="NAMidi_lex.h"
%option reentrant
%option noyywrap
%option bison-bridge
%option bison-locations
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
CHANNEL        channel
VOICE          voice
SYNTH          synth
VOLUME         volume
PAN            pan
CHORUS         chorus
REVERB         reverb
TRANSPOSE      transpose
KEY            key
DEFAULT        default
REST           r|rest

PLUS           "+"
MINUS          "-"
DIVISION       "/"
SEMICOLON      ";"
LPAREN         "("
RPAREN         ")"
LCURLY         "{"
RCURLY         "}"
COMMA          ","

INCLUDE        include

KEY_SIGN       [cdefgab](#|b)?(maj|min)
NOTE           [cdefgab](#|##|b|bb|n)?(\-?[[:digit:]]{1,2})?
IDENTIFIER     [_\-[:alnum:]]+

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
{CHANNEL}        { return CHANNEL; }
{VOICE}          { return VOICE; }
{SYNTH}          { return SYNTH; }
{VOLUME}         { return VOLUME; }
{PAN}            { return PAN; }
{CHORUS}         { return CHORUS; }
{REVERB}         { return REVERB; }
{TRANSPOSE}      { return TRANSPOSE; }
{KEY}            { return KEY; }
{DEFAULT}        { return DEFAULT; }
{REST}           { return REST; }

{PLUS}           { return PLUS; }
{MINUS}          { return MINUS; }
{DIVISION}       { return DIVISION; }

{LPAREN}         { return LPAREN; }
{RPAREN}         { return RPAREN; }
{LCURLY}         { return LCURLY; }
{RCURLY}         { return RCURLY; }
{COMMA}          { return COMMA; }

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
