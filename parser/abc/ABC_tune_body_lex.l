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
%x x_DECORATION

INLINE_FIELD  \[[+[:alpha:]]:[^\]]*\]
ANNOTATION    \"[^\"]*\"
NOTE          (^^|^|=|_|__)?[a-gA-G](,|')?[[:digit:]]*(\/[[:digit:]]*)?
BROKEN_RHYTHM <|>
REST          [xXzZ][[:digit:]]*(\/[[:digit:]]*)?

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

{NOTE}          {
                    yylval->s = strdup(yytext);
                    return NOTE;
                }

{BROKEN_RHYTHM} {
                    yylval->c = yytext[0];
                    return BROKEN_RHYTHM;
                }

{REST}          {
                    yylval->s = strdup(yytext);
                    return REST;
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
                        BEGIN(x_DECORATION);
                    }
                    else {
                        return 1;
                    }
                }

"+"             {
                    if (ABCParserIsDecoration(yyextra, '+')) {
                        yymore();
                        BEGIN(x_DECORATION);
                    }
                    else {
                        return 1;
                    }
                }

"`"             { }


\\              { BEGIN(x_ESCAPE); }
[[:space:]]     { }
.               { return 1; }

<<EOF>>         {
                    switch (YY_START) {
                    case x_ESCAPE:
                    case x_END:
                        return 0;
                    case x_DECORATION:
                        BEGIN(INITIAL); 
                        return 1;
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

<x_DECORATION>[[:graph:]]   {
                                yymore();
                                if (yytext[0] == yytext[yyleng - 1]) {
                                    BEGIN(INITIAL);
                                    yylval->s = strdup(yytext + 1);
                                    yylval->s[yyleng - 2] = '\0';
                                    return DECORATION;
                                }
                            }
<x_DECORATION>[[:space:]]   { return 1; }

%%
