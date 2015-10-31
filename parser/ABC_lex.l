%{

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

NOTE           [cdefgabCDEFGAB][,|']?[\^|=|_]{0,2}
WSPACE         [[:blank:]]+
LINE_COMMENT   %.*$


%%

<<EOF>>          { return 0; }

{NOTE}           { yylval->s = strdup(yytext); return NOTE; }
{WSPACE}         { }
{LINE_COMMENT}   { }

.                { return 1; }

%%
