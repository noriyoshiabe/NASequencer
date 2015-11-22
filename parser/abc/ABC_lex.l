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

%x s_KEY
KEY                K:
KEY_TONIC          [CDEFGAB](#|b)?|(?i:none)|(?i:hp)
KEY_MODE           [a-zA-Z]+
KEY_ACCIDENTAL     (\^{1,2}|_{1,2}|=)[cdefgabCDEFGAB]

NOTE               [cdefgabCDEFGAB][,|']?([\^|_]{1,2}|=)?

LINE_COMMENT       %.*$

%%

{WSPACE}           { }

{EOL}              { return EOL; }

{VERSION}          { yylval->s = strdup(yytext + 5); return VERSION; }
{REFERENCE_NUMBER} { yylval->s = strdup(yytext + 2); return REFERENCE_NUMBER; }
{TUNE_TITLE}       { yylval->s = strdup(yytext + 2); return TUNE_TITLE; }
{KEY}              { BEGIN(s_KEY); return KEY; }
<s_KEY>{KEY_TONIC} { yylval->s = strdup(yytext); return KEY_TONIC; }
<s_KEY>{KEY_MODE}  { yylval->s = strdup(yytext); return KEY_MODE; }
<s_KEY>{KEY_ACCIDENTAL} { yylval->s = strdup(yytext); return KEY_ACCIDENTAL; }
<s_KEY>{EOL}       { BEGIN(INITIAL); }
{NOTE}             { yylval->s = strdup(yytext); return NOTE; }

{LINE_COMMENT}     { }

.                  { return 1; }

%%
