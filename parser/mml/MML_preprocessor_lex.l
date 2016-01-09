%{
  
#include "MMLPreprocessor.h"
#include "NACString.h"
#include "NAStringBuffer.h"

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

WSPACE         [[:blank:]]+
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
<INCLUDE>\n       {
                      BEGIN(INITIAL);
                      MMLPreprocessorSyntaxError(yyextra, yylineno, yycolumn, yytext);
                      NAStringBufferAppendChar(buffer, '\n');
                  }

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

{WSPACE}          { NAStringBufferAppendString(buffer, yytext); }
{LINE_COMMENT}    { }

"/*"              { BEGIN(COMMENT); }
<COMMENT>"*/"     { BEGIN(INITIAL); }
<COMMENT>.        { }

"'"               { NAStringBufferAppendChar(buffer, '\''); BEGIN(QUOTE_STRING); }
<QUOTE_STRING>"'" { NAStringBufferAppendChar(buffer, '\''); BEGIN(INITIAL); }
<QUOTE_STRING>.   { NAStringBufferAppendChar(buffer, yytext[0]); }

"\""                      { NAStringBufferAppendChar(buffer, '"'); BEGIN(DOUBLE_QUOTE_STRING); }
<DOUBLE_QUOTE_STRING>"\"" { NAStringBufferAppendChar(buffer, '"'); BEGIN(INITIAL); }
<DOUBLE_QUOTE_STRING>.    { NAStringBufferAppendChar(buffer, yytext[0]); }

<*>\n             { NAStringBufferAppendChar(buffer, yytext[0]); }
.                 { NAStringBufferAppendChar(buffer, yytext[0]); }
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
