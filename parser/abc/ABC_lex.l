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

%x PERCENT
%x STR_INFO
%x REMARK

NONZERO             [1-9]
DIGIT               [[:digit:]]

VERSION             abc-{DIGIT}\.{DIGIT}

NOTE                [cdefgabCDEFGAB][,|']?([\^|_]{1,2}|=)?

WSPACE              [[:blank:]]+

%%

{NOTE}              { yylval->s = strdup(yytext); return NOTE; }

\n                  { return '\n'; }

{WSPACE}            { }

"%"                 { BEGIN(PERCENT); }
<PERCENT>{VERSION}  { yylval->s = strdup(yytext); return VERSION; }
<PERCENT>\n         { BEGIN(INITIAL); }
<PERCENT>.          { }

^A:                 { }
^B:                 { }
^C:                 { }
^D:                 { }
^F:                 { }
^G:                 { }
^H:                 { }
^N:                 { }
^O:                 { }
^R:                 { }
^S:                 { }
^T:                 { BEGIN(STR_INFO); return TITLE; }
^W:                 { }
^w:                 { }
^Z:                 { }

<STR_INFO>.         { yymore(); }
<STR_INFO>\n        { BEGIN(INITIAL); yylval->s = NACStringTrimWhiteSpace(strdup(yytext)); return STRING; }
 
^r:                 { BEGIN(REMARK); }
<REMARK>.           { }
<REMARK>\n          { BEGIN(INITIAL); }

.                   { return 1; }
<<EOF>>             { return 0; }

%%
