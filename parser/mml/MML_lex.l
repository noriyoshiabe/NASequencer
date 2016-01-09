%{
  
#include "MML_yacc.h"
#include "MMLParser.h"
#include "NACString.h"

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

%option prefix="MML_"
%option outfile="MML_lex.c"
%option header-file="MML_lex.h"
%option reentrant
%option noyywrap
%option bison-bridge
%option bison-locations
%option case-insensitive

%x COMMENT
%x x_INCLUDE

NONZERO        [1-9]
DIGIT          [[:digit:]]

INTEGER        {NONZERO}{DIGIT}*|0
FLOAT          ({NONZERO}{DIGIT}*|0)\.{DIGIT}+
STRING         \"[^\"]*\"|'[^']*'

WSPACE         [[:blank:]]+
LINE_COMMENT   \/\/.*$

INCLUDE        ^#include

%%

{INTEGER}         { yylval->i = atoi(yytext); return INTEGER; }
{FLOAT}           { yylval->f = atof(yytext); return FLOAT; }
{STRING}          { yylval->s = strdup(yytext + 1);
                    yylval->s[yyleng - 2] = '\0';
                    return STRING; }

{INCLUDE}         { BEGIN(x_INCLUDE); }
<x_INCLUDE>{STRING} {
                      char *filepath = NACStringDuplicate(yytext + 1);
                      filepath[yyleng - 2] = '\0';
                      MMLParserParseIncludeFile(yyextra, yylineno, yycolumn, filepath);
                      BEGIN(INITIAL);
                  }
<x_INCLUDE>\n     { BEGIN(INITIAL); return -1; }

{WSPACE}          { }
{LINE_COMMENT}    { }

"/*"              { BEGIN(COMMENT); }
<COMMENT>"*/"     { BEGIN(INITIAL); }
<COMMENT>.        { }

.                 { return 1; }
<<EOF>>           {
                      if (!MMLParserPopPreviousFile(yyextra)) {
                          return 0;
                      }
                  }
 
%%
