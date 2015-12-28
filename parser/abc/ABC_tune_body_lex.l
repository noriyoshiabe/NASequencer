%{
  
#include "ABCParser.h"
#include "ABC_tune_body_yacc.h"
#include <string.h>
#include <NACString.h>

#define YY_USER_INIT \
    yycolumn = 1;

#define YY_USER_ACTION \
    yylloc->first_column = yycolumn; \
    if (strchr(yytext, '\n')) yycolumn = 1; \
    else yycolumn += yyleng;

%}

%option prefix="ABC_tune_body_"
%option outfile="ABC_tune_body_lex.c"
%option header-file="ABC_tune_body_lex.h"
%option reentrant
%option noyywrap
%option bison-bridge
%option bison-locations

INLINE_FIELD \[[+[:alpha:]]:[^\]]*\]

%%

{INLINE_FIELD}  {
                    yylval->s = strdup(yytext + 1);
                    yylval->s[yyleng - 2] = '\0';
                    return INLINE_FIELD;
                }
.               { ECHO; }

<<EOF>>         { return 0; }

%%
