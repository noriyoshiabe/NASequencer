%{
  
#include "ABCParser.h"
#include "ABC_yacc.h"
#include <string.h>
#include <NACString.h>

#define YY_USER_INIT \
    yylineno = 1; \
    yycolumn = 1;

#define YY_USER_ACTION \
    yylloc->first_line = yylineno; \
    yylloc->first_column = yycolumn; \
    if (strchr(yytext, '\n')) ++yylineno, yycolumn = 1; \
    else yycolumn += yyleng;

%}

%option prefix="ABC_"
%option outfile="ABC_lex.c"
%option header-file="ABC_lex.h"
%option reentrant
%option noyywrap
%option bison-bridge
%option bison-locations

%x x_TUNE

FILE_IDENTIFICATION ^%abc(-[[:digit:]]\.[[:digit:]])?[[:blank:]]*$
REFERNCE_NUMBER     ^X:.*$
INFORMATION         ^[+[:alpha:]]:.*$
DIRECTIVE           ^%%.*$
EMPTY_LINE          ^[[:blank:]]*\n
TUNE_BODY           ^[[:print:]]*$

%%

{FILE_IDENTIFICATION} { yylval->s = strdup(yytext); return FILE_IDENTIFICATION; }
{REFERNCE_NUMBER}     { BEGIN(x_TUNE); yylval->s = strdup(yytext); return INFORMATION; }

<*>{INFORMATION}      { yylval->s = strdup(yytext); return INFORMATION; }
<*>{DIRECTIVE}        { yylval->s = strdup(yytext); return DIRECTIVE; }

<x_TUNE>{EMPTY_LINE}  { BEGIN(INITIAL); }
<x_TUNE>{TUNE_BODY}   { yylval->s = strdup(yytext); return TUNE_BODY; }

<*>.                  { }
<*>\r                 { }
<*>\n                 { }

<<EOF>>               { return 0; }

%%
