%{
  
#include "NAMidiPreprocessor.h"
#include "NACString.h"

#include <string.h>

#define YY_USER_INIT \
    yylineno = 1; \
    yycolumn = 1;

#define YY_USER_ACTION \
    if (strchr(yytext, '\n')) ++yylineno, yycolumn = 1; \
    else yycolumn += yyleng; \
    \
    NAMidiPreprocessorConsumeExpandingChar(yyextra);\

%}

%option prefix="NAMidi_preprocessor_"
%option outfile="NAMidi_preprocessor_lex.c"
%option header-file="NAMidi_preprocessor_lex.h"
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
LINE_COMMENT   [[:blank:]]*(\/\/|==|--).*$

INCLUDE        ^[[:blank:]]*#[[:blank:]]*include
DEFINE         ^[[:blank:]]*#[[:blank:]]*define
MACRO_SYMBOL   [_[:alpha:]][_[:alnum:]#\+-]*
MACRO_ARGS     \([[:blank:]]*{MACRO_SYMBOL}([[:blank:]]*,[[:blank:]]*{MACRO_SYMBOL})*[[:blank:]]*\)
MACRO_DEFINE   {MACRO_SYMBOL}([[:blank:]]*{MACRO_ARGS})?[[:blank:]].*$
MACRO_REF      {MACRO_SYMBOL}([[:blank:]]*\([^\(\)]*\))?

%%

{INCLUDE}         { BEGIN(INCLUDE); }
<INCLUDE>{STRING} {
                      char *filepath = NACStringDuplicate(yytext + 1);
                      filepath[yyleng - 2] = '\0';
                      NAMidiPreprocessorIncludeFile(yyextra, yylineno, yycolumn, filepath);
                      BEGIN(INITIAL);
                  }
<INCLUDE>\n       {
                      BEGIN(INITIAL);
                      NAMidiPreprocessorSyntaxError(yyextra, yylineno, yycolumn, yytext);
                      fputc('\n', stream);
                  }

{DEFINE}          { yymore(); BEGIN(MACRO); }
<MACRO>{MACRO_DEFINE} { NAMidiPreprocessorAppendMacro(yyextra, yylineno, yycolumn, yytext); }
<MACRO>\n             { BEGIN(INITIAL);  }
<MACRO>.              { }

{MACRO_REF}       {
                      char *expanded = NAMidiPreprocessorExpandMacro(yyextra, yylineno, yycolumn, yytext);
                      if (expanded) {
                          char *pc = expanded + strlen(expanded);
                          while (pc > expanded) {
                              unput(*--pc);
                          }
                          free(expanded);
                      }
                      else {
                          fputs(yytext, stream);
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
                          NAMidiPreprocessorUnexpectedEOF(yyextra, yylineno, yycolumn);
                          BEGIN(INITIAL);
                      }

                      if (!NAMidiPreprocessorPopPreviousFile(yyextra)) {
                          return 0;
                      }
                  }
 
%%
