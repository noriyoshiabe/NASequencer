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

#define YYMORE() yymore(), yycolumn -= yyleng
#define ECHO

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
%x x_CHORD

INLINE_FIELD  \[[+[:alpha:]]:[^\]]*\]
ANNOTATION    \"[^\"]*\"
NOTE          (^^|^|=|_|__)?[a-gA-G](,|')*[[:digit:]]*(\/[[:digit:]]*)*
BROKEN_RHYTHM <{1,3}|>{1,3}
REST          [xXzZ][[:digit:]]*(\/[[:digit:]]*)*
REPEAT_BAR    [\|:\[\]]+([[:digit:]]+[-,[:digit:]]*)?
ACCIACCATURA  \{\/
TUPLET        \([[:digit:]]+(:[[:digit:]]*)?(:[[:digit:]]*)?
CHORD         \[[^\]\[]+\][[:digit:]]*(\/[[:digit:]]*)?
CHORD_END     \][[:digit:]]*(\/[[:digit:]]*)?
EXTRA_SPACE   [y][[:digit:]]*
RESERVED      [#\*;\?@]

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
                    yylval->s = yytext;
                    return BROKEN_RHYTHM;
                }

{REST}          {
                    yylval->s = strdup(yytext);
                    return REST;
                }

{REPEAT_BAR}    {
                    yylval->s = strdup(yytext);
                    return REPEAT_BAR;
                }

{ACCIACCATURA}  { return ACCIACCATURA; }

{TUPLET}        {
                    yylval->s = strdup(yytext);
                    return TUPLET;
                }

{CHORD}         {
                    yycolumn -= yyleng;
                    yyless(0);
                    BEGIN(x_CHORD);
                }

{EXTRA_SPACE}   { return EXTRA_SPACE; }
{RESERVED}      { return RESERVED; }

"&"             { return '&'; }
"-"             { return '-'; }
"("             { return '('; }
")"             { return ')'; }
"{"             { return '{'; }
"}"             { return '}'; }
"."             { return '.'; }

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
                        YYMORE();
                        BEGIN(x_DECORATION);
                    }
                    else {
                        return 1;
                    }
                }

"+"             {
                    if (ABCParserIsDecoration(yyextra, '+')) {
                        YYMORE();
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
                                if (yytext[0] == yytext[yyleng - 1]) {
                                    BEGIN(INITIAL);
                                    yylval->s = strdup(yytext + 1);
                                    yylval->s[yyleng - 2] = '\0';
                                    return DECORATION;
                                }
                                else {
                                    YYMORE();
                                }
                            }
<x_DECORATION>[[:space:]]   { return 1; }

<x_CHORD>"["              { return '['; }
<x_CHORD>{NOTE}           {
                              yylval->s = strdup(yytext);
                              return NOTE;
                          }
<x_CHORD>"."              { }
<x_CHORD>!.+!             { }
<x_CHORD>{CHORD_END}      {
                              BEGIN(INITIAL);
                              yylval->s = strdup(yytext + 1);
                              return CHORD_END;
                          }
<x_CHORD>.                { return 1; }

%%
