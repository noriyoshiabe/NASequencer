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
%x STR_INFO
%x INST_INFO
%x REMARK
%x ERROR

NONZERO             [1-9]
DIGIT               [[:digit:]]

INTEGER             {NONZERO}{DIGIT}*|0

VERSION             %abc-{DIGIT}\.{DIGIT}
INCLUDE             abc-include

FILEPATH            [0-9a-zA-Z_\-]+\.abh
QUOTED_FILEPATH     '[0-9a-zA-Z_\- ]+\.abh'|"[0-9a-zA-Z_\- ]+\.abh"
NOTE                [cdefgabCDEFGAB][,|']?([\^|_]{1,2}|=)?

KEY_TONIC           {PITCH}(#|b)?|(?i:none)|(?i:hp)
KEY_MODE            (?i:m(aj(or)?)?|min(or)?|ion(ian)?|aeo(lian)?|mix(olydian)?|dor(ian)?|phr(ygian)?|lyd(ian)?|loc(rian)?|exp(licit)?)
KEY_ACCIDENTAL      (\^{1,2}|_{1,2}|=)[cdefgabCDEFGAB]
CLEF                (?i:clef)
CLEF_NAME           (?i:treble|alto|tenor|bass|perc|none)[1-5]?([\+-]8)?
MIDDLE              (?i:middle)
PITCH               (?i:[cdefgab])
TRANSPOSE           (?i:t(ranspose)?)
OCTAVE              (?i:octave)
STAFF_LINES         (?i:stafflines)

WSPACE              [[:blank:]]+

%%

{NOTE}              { yylval->s = strdup(yytext); return NOTE; }

\n                  { return '\n'; }

{WSPACE}            { }

{VERSION}           { yylval->s = strdup(yytext); return VERSION; }

^A:                 { }
^B:                 { }
^C:                 { }
^D:                 { }
^F:                 { }
^G:                 { }
^H:                 { }
^N:                 { }
^O:                 { }
^R:                 { }
^S:                 { }
^T:                 { BEGIN(STR_INFO); return TITLE; }
^W:                 { }
^w:                 { }
^Z:                 { }
<STR_INFO>.         { yymore(); }
<STR_INFO>\n        { BEGIN(INITIAL); yylval->s = NACStringTrimWhiteSpace(strdup(yytext)); return STRING; }

^I:                          { BEGIN(INST_INFO); return INSTRUCTION; }
^K:                          { BEGIN(INST_INFO); return KEY; }
^X:                          { BEGIN(INST_INFO); return REFERENCE_NUMBER; }
<INST_INFO>{WSPACE}          { }
<INST_INFO>{INCLUDE}         { return INCLUDE; }
<INST_INFO>{INTEGER}         { yylval->i = atoi(yytext); return INTEGER; }
<INST_INFO>{PITCH}           { yylval->s = strdup(yytext); return PITCH; }
<INST_INFO>{KEY_TONIC}       { yylval->s = strdup(yytext); return KEY_TONIC; }
<INST_INFO>{KEY_MODE}        { yylval->s = strdup(yytext); return KEY_MODE; }
<INST_INFO>{KEY_ACCIDENTAL}  { yylval->s = strdup(yytext); return KEY_ACCIDENTAL; }
<INST_INFO>{CLEF}            { return CLEF; }
<INST_INFO>{CLEF_NAME}       { yylval->s = strdup(yytext); return CLEF_NAME; }
<INST_INFO>{MIDDLE}          { return MIDDLE; }
<INST_INFO>{TRANSPOSE}       { return TRANSPOSE; }
<INST_INFO>{OCTAVE}          { return OCTAVE; }
<INST_INFO>{STAFF_LINES}     { return STAFF_LINES; }
<INST_INFO>{FILEPATH}        { yylval->s = strdup(yytext); return FILEPATH; }
<INST_INFO>{QUOTED_FILEPATH} { yylval->s = strdup(yytext + 1); yylval->s[strlen(yylval->s) - 1] = '\0'; return FILEPATH; }
<INST_INFO>"-"               { return '-'; }
<INST_INFO>"+"               { return '+'; }
<INST_INFO>"="               { return '='; }
<INST_INFO>\n                { BEGIN(INITIAL); }

"%"                 { BEGIN(COMMENT); }
<COMMENT>\n         { BEGIN(INITIAL); }
<COMMENT>.          { }

^r:                 { BEGIN(REMARK); }
<REMARK>.           { }
<REMARK>\n          { BEGIN(INITIAL); }

<ERROR>\n           { yy_pop_state(yyscanner); BEGIN(INITIAL); }
<ERROR>.            { }

<*>.                { return 1; }
<<EOF>>             { return 0; }

%%

void ABC_lex_set_error_until_eol_if_needed(yyscan_t scanner)
{
    struct yyguts_t *yyg = (struct yyguts_t *)scanner;
    if (INST_INFO == YY_START) {
        yy_push_state(ERROR, scanner);
    }
}
