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

STRING         \"[^\"]*\"|'[^']*'

LINE_COMMENT   \/\/.*$

INCLUDE        ^[[:blank:]]*#[[:blank:]]*include
MACRO_SYMBOL   \$[_[:alpha:]][_[:alnum:]#\+\(\)]*
MACRO_DEFINE   {MACRO_SYMBOL}[[:blank:]]*=.*;
MACRO_ARGS     \{[^,]+(,[^,]+)*\}
MACRO_REF      {MACRO_SYMBOL}([[:blank:]]*{MACRO_ARGS})?

%%

{INCLUDE}         { BEGIN(INCLUDE); }
<INCLUDE>{STRING} {
                      char *filepath = NACStringDuplicate(yytext + 1);
                      filepath[yyleng - 2] = '\0';
                      MMLPreprocessorIncludeFile(yyextra, yylineno, yycolumn, filepath);
                      BEGIN(INITIAL);
                  }
<INCLUDE>\n       { BEGIN(INITIAL); /* TODO ERROR */ }

{MACRO_DEFINE}    { MMLPreprocessorAppendMacro(yyextra, yylineno, yycolumn, yytext); }
{MACRO_REF}       {
                      char *expanded = MMLPreprocessorExpandMacro(yyextra, yylineno, yycolumn, yytext);
                      if (expanded) {
                          char *pc = expanded + strlen(expanded);
                          while (pc > expanded) {
                              unput(*--pc);
                          }
                      }
                  }

{LINE_COMMENT}    { }

"/*"              { BEGIN(COMMENT); }
<COMMENT>"*/"     { BEGIN(INITIAL); }
<COMMENT>.*       { }

"'"               { BEGIN(QUOTE_STRING); }
<QUOTE_STRING>"'" { BEGIN(INITIAL); }
<QUOTE_STRING>.*  { }

"\""                      { BEGIN(DOUBLE_QUOTE_STRING); }
<DOUBLE_QUOTE_STRING>"\"" { BEGIN(INITIAL); }
<DOUBLE_QUOTE_STRING>.*   { }

.*                { }
<<EOF>>           {
                      // TODO ERROR IF NOT INITIAL

                      if (!MMLPreprocessorPopPreviousFile(yyextra)) {
                          return 0;
                      }
                  }
 
%%
