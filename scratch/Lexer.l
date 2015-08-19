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

PLUS           "+"
MINUS          "-"
ASSIGN         "="
COMMA          ","
SEMICOLON      ";"

IDENTIFIER     $[_\-[:alnum:]]+

EOL            \n

LINE_COMMENT   \/\/.*$

%%

<<EOF>>          { return 0; }

{INTEGER}        { yylval->i = atoi(yytext); return INTEGER; }
{FLOAT}          { yylval->f = atof(yytext); return FLOAT; }
{STRING}         { yylval->s = strdup(yytext); return STRING; }
{WSPACE}         { }

{RESOLUTION}     { return RESOLUTION; }
{TITLE}          { return TITLE; }

{PLUS}           { return PLUS; }
{MINUS}          { return MINUS; }
{ASSIGN}         { return ASSIGN; }
{COMMA}          { return COMMA; }

{IDENTIFIER}     { yylval->s = strdup(yytext); return IDENTIFIER; }

{EOL}            { return EOL; }

{LINE_COMMENT}   { }
"/*"             { BEGIN(COMMENT); }
<COMMENT>"*/"    { BEGIN(INITIAL); }
<COMMENT>\n      { }
<COMMENT>.       { }

.                { return 1; }
 
%%
