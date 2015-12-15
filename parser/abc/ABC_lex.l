%{
  
#include "ABCParser.h"
#include "ABC_yacc.h"
#include <string.h>
#include <NACString.h>

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
%option stack

%x COMMENT
%x ERROR

%x x_STR_INFO
%x x_INSTRUCTION
%x x_KEY
%x x_METER
%x x_REFERENCE_NUMBER
%x x_REMARK

NONZERO             [1-9]
DIGIT               [[:digit:]]

INTEGER             {NONZERO}{DIGIT}*|0

VERSION             %abc-{DIGIT}\.{DIGIT}
INCLUDE             abc-include

FILEPATH            [0-9a-zA-Z_\-]+\.abh
QUOTED_FILEPATH     '[0-9a-zA-Z_\- ]+\.abh'|"[0-9a-zA-Z_\- ]+\.abh"
NOTE                [cdefgabCDEFGAB][,|']?([\^|_]{1,2}|=)?

PITCH               (?i:[cdefgab])
NONE                (?i:none)
KEY_TONIC           {PITCH}(#|b)?|(?i:hp)
KEY_MODE            (?i:m(aj(or)?)?|min(or)?|ion(ian)?|aeo(lian)?|mix(olydian)?|dor(ian)?|phr(ygian)?|lyd(ian)?|loc(rian)?|exp(licit)?)
KEY_ACCIDENTAL      (\^{1,2}|_{1,2}|=)[cdefgabCDEFGAB]
CLEF                (?i:clef)
CLEF_NAME           (?i:treble|alto|tenor|bass|perc|none)[1-5]?([\+-]8)?
MIDDLE              (?i:middle)
TRANSPOSE           (?i:t(ranspose)?)
OCTAVE              (?i:octave)
STAFF_LINES         (?i:stafflines)

WSPACE              [[:blank:]]+

%%

{NOTE}              { yylval->s = strdup(yytext); return NOTE; }

\n                  { return '\n'; }

{WSPACE}            { }

{VERSION}           { yylval->s = strdup(yytext); return VERSION; }

^A:                 { BEGIN(x_STR_INFO); yylval->c = 'A'; return STRING_INFORMATION; }
^B:                 { BEGIN(x_STR_INFO); yylval->c = 'B'; return STRING_INFORMATION; }
^C:                 { BEGIN(x_STR_INFO); yylval->c = 'C'; return STRING_INFORMATION; }
^D:                 { BEGIN(x_STR_INFO); yylval->c = 'D'; return STRING_INFORMATION; }
^F:                 { BEGIN(x_STR_INFO); yylval->c = 'F'; return STRING_INFORMATION; }
^G:                 { BEGIN(x_STR_INFO); yylval->c = 'G'; return STRING_INFORMATION; }
^H:                 { BEGIN(x_STR_INFO); yylval->c = 'H'; return STRING_INFORMATION; }
^N:                 { BEGIN(x_STR_INFO); yylval->c = 'N'; return STRING_INFORMATION; }
^O:                 { BEGIN(x_STR_INFO); yylval->c = 'O'; return STRING_INFORMATION; }
^R:                 { BEGIN(x_STR_INFO); yylval->c = 'R'; return STRING_INFORMATION; }
^S:                 { BEGIN(x_STR_INFO); yylval->c = 'S'; return STRING_INFORMATION; }
^T:                 { BEGIN(x_STR_INFO); return TITLE; }
^W:                 { BEGIN(x_STR_INFO); yylval->c = 'W'; return STRING_INFORMATION; }
^w:                 { BEGIN(x_STR_INFO); yylval->c = 'w'; return STRING_INFORMATION; }
^Z:                 { BEGIN(x_STR_INFO); yylval->c = 'Z'; return STRING_INFORMATION; }
<x_STR_INFO>.       { yymore(); }
<x_STR_INFO>\n      { BEGIN(INITIAL); yylval->s = NACStringTrimWhiteSpace(strdup(yytext)); return STRING; }

^I:                                { BEGIN(x_INSTRUCTION); return INSTRUCTION; }
<x_INSTRUCTION>{WSPACE}            { }
<x_INSTRUCTION>{INCLUDE}           { return INCLUDE; }
<x_INSTRUCTION>{FILEPATH}          { yylval->s = strdup(yytext); return FILEPATH; }
<x_INSTRUCTION>{QUOTED_FILEPATH}   { yylval->s = strdup(yytext + 1); yylval->s[strlen(yylval->s) - 1] = '\0'; return FILEPATH; }
<x_INSTRUCTION>"%"                 { BEGIN(COMMENT); }
<x_INSTRUCTION>\n                  { BEGIN(INITIAL); }

^K:                      { BEGIN(x_KEY); return KEY; }
<x_KEY>{WSPACE}          { }
<x_KEY>{INTEGER}         { yylval->i = atoi(yytext); return INTEGER; }
<x_KEY>{PITCH}           { yylval->s = strdup(yytext); return PITCH; }
<x_KEY>{NONE}            { yylval->s = strdup(yytext); return NONE; }
<x_KEY>{KEY_TONIC}       { yylval->s = strdup(yytext); return KEY_TONIC; }
<x_KEY>{KEY_MODE}        { yylval->s = strdup(yytext); return KEY_MODE; }
<x_KEY>{KEY_ACCIDENTAL}  { yylval->s = strdup(yytext); return KEY_ACCIDENTAL; }
<x_KEY>{CLEF}            { return CLEF; }
<x_KEY>{CLEF_NAME}       { yylval->s = strdup(yytext); return CLEF_NAME; }
<x_KEY>{MIDDLE}          { return MIDDLE; }
<x_KEY>{TRANSPOSE}       { return TRANSPOSE; }
<x_KEY>{OCTAVE}          { return OCTAVE; }
<x_KEY>{STAFF_LINES}     { return STAFF_LINES; }
<x_KEY>"-"               { return '-'; }
<x_KEY>"+"               { return '+'; }
<x_KEY>"="               { return '='; }
<x_KEY>"%"               { BEGIN(COMMENT); }
<x_KEY>\n                { BEGIN(INITIAL); }

^M:                    { BEGIN(x_METER); return METER; }
<x_METER>{WSPACE}        { }
<x_METER>{INTEGER}       { yylval->i = atoi(yytext); return INTEGER; }
<x_METER>{NONE}          { yylval->s = strdup(yytext); return NONE; }
<x_METER>"-"             { return '-'; }
<x_METER>"+"             { return '+'; }
<x_METER>"/"             { return '/'; }
<x_METER>"("             { return '('; }
<x_METER>")"             { return ')'; }
<x_METER>"%"             { BEGIN(COMMENT); }
<x_METER>\n              { BEGIN(INITIAL); }

^X:                           { BEGIN(x_REFERENCE_NUMBER); return REFERENCE_NUMBER; }
<x_REFERENCE_NUMBER>{WSPACE}  { }
<x_REFERENCE_NUMBER>{INTEGER} { yylval->i = atoi(yytext); return INTEGER; }
<x_REFERENCE_NUMBER>"%"       { BEGIN(COMMENT); }
<x_REFERENCE_NUMBER>\n        { BEGIN(INITIAL); }

^r:                 { BEGIN(x_REMARK); }
<x_REMARK>.         { }
<x_REMARK>\n        { BEGIN(INITIAL); }

"%"                 { BEGIN(COMMENT); }
<COMMENT>\n         { BEGIN(INITIAL); }
<COMMENT>.          { }

<ERROR>\n           { yy_pop_state(yyscanner); BEGIN(INITIAL); }
<ERROR>.            { }

<*>.                { return 1; }
<<EOF>>             { return 0; }

%%

void ABC_lex_set_error_until_eol_if_needed(yyscan_t scanner)
{
    struct yyguts_t *yyg = (struct yyguts_t *)scanner;
    if (INITIAL != YY_START) {
        yy_push_state(ERROR, scanner);
    }
}
