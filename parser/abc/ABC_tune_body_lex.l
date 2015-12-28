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

%x x_ESCAPE
%x x_COMMENT
%x x_END

INLINE_FIELD \[[+[:alpha:]]:[^\]]*\]
ANNOTATION   \"[^\"]*\"

%%

{INLINE_FIELD}  {
                    yylval->s = strdup(yytext + 1);
                    yylval->s[yyleng - 2] = '\0';
                    return INLINE_FIELD;
                }

{ANNOTATION}    {
                    yylval->s = strdup(yytext + 1);
                    yylval->s[yyleng - 2] = '\0';
                    return ANNOTATION;
                }

"%"             { BEGIN(x_COMMENT); }

"$"             {
                    if (ABCParserIsLineBreak(yyextra, '$')) {
                        return '\n';
                    }
                    else {
                        return 1;
                    }
                }

"!"             {
                    if (ABCParserIsLineBreak(yyextra, '!')) {
                        return '\n';
                    }
                    else if (ABCParserIsDecoration(yyextra, '!')) {
                        yymore();
                        //BEGIN(x_DECORATION);
                    }
                    else {
                        return 1;
                    }
                }

"+"             {
                    if (ABCParserIsDecoration(yyextra, '+')) {
                        yymore();
                        //BEGIN(x_DECORATION);
                    }
                    else {
                        return 1;
                    }
                }

\\              { BEGIN(x_ESCAPE); }
.               { }

<<EOF>>         {
                    switch (YY_START) {
                    case x_ESCAPE:
                    case x_END:
                        return 0;
                    default:
                        if (ABCParserIsLineBreak(yyextra, '\n')) {
                            BEGIN(x_END);
                            return '\n';
                        }
                        else {
                            return 0;
                        }
                    }
                }

<x_ESCAPE>"%"         { BEGIN(INITIAL); return '%'; }
<x_ESCAPE>[[:space:]] { }
<x_ESCAPE>.           { BEGIN(INITIAL); return 1; }

<x_COMMENT>.          { }

%%
