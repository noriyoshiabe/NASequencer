%{
  
#include "Expression.h"
#include "YYContext.h"
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

NOTE           [cdefgab](\+{1,2}|\-{1,2}|=)?(_|^)?
KEY_SIGN       [cdefgab](\+|\-)?(maj|min)
OCTAVE_SHIFT   <|>

KEY            key
TIME           time
TEMPO          tempo
MARKER         marker
CHANNEL        channel
VELOCITY       velocity
GATETIME       gatetime
AUTO           auto
OCTAVE         octave

REST           "r"
TIE            "~"

LENGTH         length
OFFSET         offset
LOCATION       location

PLUS           "+"
MINUS          "-"
MULTIPLY       "*"
DIVISION       "/"
ASSIGN         "="
COMMA          ","
DOT            "."
LCURLY         "{"
RCURLY         "}"

IDENTIFIER     $[_\-[:alnum:]]+

EOL            \n

LINE_COMMENT   \/\/.*$

%%

<<EOF>>          {
                   YYContext *context = yyget_extra(yyscanner);
                   if (context->eofReached) {
                     return 0;
                   } else {
                     context->eofReached = true;
                     return __EOF__;
                   }
                 }

{INTEGER}        { yylval->i = atoi(yytext); return INTEGER; }
{FLOAT}          { yylval->f = atof(yytext); return FLOAT; }
{STRING}         { yylval->s = strdup(yytext); return STRING; }
{WSPACE}         { }

{NOTE}           { yylval->s = strdup(yytext); return NOTE; }
{KEY_SIGN}       { yylval->s = strdup(yytext); return KEY_SIGN; }
{OCTAVE_SHIFT}   { yylval->s = strdup(yytext); return OCTAVE_SHIFT; }

{KEY}            { return KEY; }
{TIME}           { return TIME; }
{TEMPO}          { return TEMPO; }
{MARKER}         { return MARKER; }
{CHANNEL}        { return CHANNEL; }
{VELOCITY}       { return VELOCITY; }
{GATETIME}       { return GATETIME; }
{AUTO}           { return AUTO; }
{OCTAVE}         { return OCTAVE; }

{REST}           { return REST; }
{TIE}            { return TIE; }

{LENGTH}         { return LENGTH; }
{OFFSET}         { return OFFSET; }
{LOCATION}       { return LOCATION; }

{PLUS}           { return PLUS; }
{MINUS}          { return MINUS; }
{MULTIPLY}       { return MULTIPLY; }
{DIVISION}       { return DIVISION; }
{ASSIGN}         { return ASSIGN; }
{COMMA}          { return COMMA; }
{DOT}            { return DOT; }
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
