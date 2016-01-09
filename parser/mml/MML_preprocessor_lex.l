%{
  
#include "MMLPreprocessor.h"
#include "NACString.h"

#include <string.h>

#define YY_USER_INIT \
    yylineno = 1; \
    yycolumn = 1;

#define YY_USER_ACTION \
    if (strchr(yytext, '\n')) ++yylineno, yycolumn = 1; \
    else yycolumn += yyleng;

%}

%option prefix="MML_preprocessor_"
%option outfile="MML_preprocessor_lex.c"
%option header-file="MML_preprocessor_lex.h"
%option reentrant
%option noyywrap
%option case-insensitive

%x COMMENT
%x INCLUDE
%x QUOTE_STRING
%x DOUBLE_QUOTE_STRING
%x MACRO

STRING         \"[^\"]*\"|'[^']*'

WSPACE         [[:blank:]]+
LINE_COMMENT   \/\/.*$

INCLUDE        ^[[:blank:]]*#[[:blank:]]*include
MACRO_STRING   [_[:alpha:]][_[:alnum:]#\+\(\)]*
MACRO_SYMBOL   \${MACRO_STRING}
MACRO_ARGS     \{[[:blank:]]*{MACRO_STRING}([[:blank:]]*,[[:blank:]]*{MACRO_STRING})*[[:blank:]]*\}
MACRO_DEFINE   {MACRO_SYMBOL}([[:blank:]]*{MACRO_ARGS})?[[:blank:]]*=
MACRO_REF      {MACRO_SYMBOL}([[:blank:]]*{MACRO_ARGS})?

%%

{INCLUDE}         { BEGIN(INCLUDE); }
<INCLUDE>{STRING} {
                      char *filepath = NACStringDuplicate(yytext + 1);
                      filepath[yyleng - 2] = '\0';
                      MMLPreprocessorIncludeFile(yyextra, yylineno, yycolumn, filepath);
                      BEGIN(INITIAL);
                  }
<INCLUDE>\n       {
                      BEGIN(INITIAL);
                      MMLPreprocessorSyntaxError(yyextra, yylineno, yycolumn, yytext);
                      fputc('\n', stream);
                  }

{MACRO_DEFINE}    { yymore(); BEGIN(MACRO); }
<MACRO>;          { BEGIN(INITIAL); MMLPreprocessorAppendMacro(yyextra, yylineno, yycolumn, yytext); }
<MACRO>\n         { fputc(yytext[0], stream); yymore(); }
<MACRO>.          { yymore(); }
{MACRO_REF}       {
                      char *expanded = MMLPreprocessorExpandMacro(yyextra, yylineno, yycolumn, yytext);
                      if (expanded) {
                          char *pc = expanded + strlen(expanded);
                          while (pc > expanded) {
                              unput(*--pc);
                          }
                          free(expanded);
                      }
                  }

{WSPACE}          { fputs(yytext, stream); }
{LINE_COMMENT}    { }

"/*"              { BEGIN(COMMENT); }
<COMMENT>"*/"     { BEGIN(INITIAL); }
<COMMENT>.        { }

"'"               { fputc('\'', stream); BEGIN(QUOTE_STRING); }
<QUOTE_STRING>"'" { fputc('\'', stream); BEGIN(INITIAL); }
<QUOTE_STRING>.   { fputc(yytext[0], stream); }

"\""                      { fputc('"', stream); BEGIN(DOUBLE_QUOTE_STRING); }
<DOUBLE_QUOTE_STRING>"\"" { fputc('"', stream); BEGIN(INITIAL); }
<DOUBLE_QUOTE_STRING>.    { fputc(yytext[0], stream); }

<*>\n             { fputc(yytext[0], stream); }
.                 { fputc(yytext[0], stream); }
<<EOF>>           {
                      if (YYSTATE != INITIAL) {
                          MMLPreprocessorUnexpectedEOF(yyextra, yylineno, yycolumn);
                          BEGIN(INITIAL);
                      }

                      if (!MMLPreprocessorPopPreviousFile(yyextra)) {
                          return 0;
                      }
                  }
 
%%
