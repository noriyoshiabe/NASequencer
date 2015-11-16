%{
  
#include "ABCParser.h"
#include "ABC_yacc.h"
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

%option prefix="ABC_"
%option outfile="ABC_lex.c"
%option header-file="ABC_lex.h"
%option reentrant
%option noyywrap
%option bison-bridge
%option bison-locations

WSPACE             [[:blank:]]+

EOL                \n

VERSION            %abc-.+$
REFERENCE_NUMBER   X:.*$
TUNE_TITLE         T:.*$
NOTE               [cdefgabCDEFGAB][,|']?([\^|_]{1,2}|=)?

LINE_COMMENT       %.*$

%%

{WSPACE}           { }

{EOL}              { return EOL; }

{VERSION}          { yylval->s = strdup(yytext + 5); return VERSION; }
{REFERENCE_NUMBER} { yylval->s = strdup(yytext + 2); return REFERENCE_NUMBER; }
{TUNE_TITLE}       { yylval->s = strdup(yytext + 2); return TUNE_TITLE; }
{NOTE}             { yylval->s = strdup(yytext); return NOTE; }

{LINE_COMMENT}     { }

.                  { return 1; }

%%
