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
VOLUME         volume
PAN            pan
CHORUS         chorus
REVERB         reverb
TRANSPOSE      transpose
KEY            key

PLUS           "+"
MINUS          "-"
DIVISION       "/"
SEMICOLON      ";"

INCLUDE        include

IDENTIFIER     [^@$;"=\^%!\`\n[:blank:]]+
PATTERN_ID     @{IDENTIFIER}
PHRASE_ID      ${IDENTIFIER}
KEY_SIGN       [cdefgab](#|b)?(maj|min)
NOTE           [cdefgab](#|b|n)?\-?[[:digit:]]{1,2}

EOL            \n

LINE_COMMENT   (\/\/|==|--).*$

%%

<<EOF>>          { return 0; }

{INTEGER}        { yylval->i = atoi(yytext); return INTEGER; }
{FLOAT}          { yylval->f = atof(yytext); return FLOAT; }
{STRING}         { yylval->s = yytext; return STRING; }
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
{VOLUME}         { return VOLUME; }
{PAN}            { return PAN; }
{CHORUS}         { return CHORUS; }
{REVERB}         { return REVERB; }
{TRANSPOSE}      { return TRANSPOSE; }
{KEY}            { return KEY; }

{PLUS}           { return PLUS; }
{MINUS}          { return MINUS; }
{DIVISION}       { return DIVISION; }

{PATTERN_ID}     { yylval->s = yytext + 1; return PATTERN_ID; }
{PHRASE_ID}      { yylval->s = yytext + 1; return PHRASE_ID; }
{NOTE}           { yylval->s = yytext; return NOTE; }
{KEY_SIGN}       { yylval->s = yytext; return KEY_SIGN; }

{EOL}            { return EOL; }
{SEMICOLON}      { return SEMICOLON; }
{INCLUDE}        { return INCLUDE; }

{LINE_COMMENT}   { }
"/*"             { BEGIN(COMMENT); }
<COMMENT>"*/"    { BEGIN(INITIAL); }
<COMMENT>\n      { }
<COMMENT>.       { }

.                { return 1; }
 
%%
