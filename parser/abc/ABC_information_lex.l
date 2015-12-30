%{
  
#include "ABCParser.h"
#include "ABC_information_yacc.h"
#include <string.h>
#include <NACString.h>

#define YY_USER_INIT \
    yycolumn = 1;

#define YY_USER_ACTION \
    yylloc->first_column = yycolumn; \
    if (strchr(yytext, '\n')) yycolumn = 1; \
    else yycolumn += yyleng;

%}

%option prefix="ABC_information_"
%option outfile="ABC_information_lex.c"
%option header-file="ABC_information_lex.h"
%option reentrant
%option noyywrap
%option bison-bridge
%option bison-locations

%x ERROR
%x COMMENT

%x x_STR_INFO
%x x_INSTRUCTION
%x x_INST_INCLUDE
%x x_INST_CHARSET
%x x_INST_VERSION
%x x_INST_CREATOR
%x x_INST_LINEBREAK
%x x_INST_DECORATION
%x x_KEY
%x x_METER
%x x_UNIT_NOTE_LENGTH
%x x_TEMPO
%x x_PARTS
%x x_REFERENCE_NUMBER
%x x_MACRO
%x x_MACRO_REPLACEMENT
%x x_REDEFINABLE_SYMBOL
%s x_SYMBOL_LINE
%x x_REMARK
%x x_CONTINUATION
%x x_VOICE
%x x_VOICE_PARAM

NONZERO             [1-9]
DIGIT               [[:digit:]]
INTEGER             {NONZERO}{DIGIT}*|0
FLOAT               ({NONZERO}{DIGIT}*|0)\.{DIGIT}+
QUOTED_STRING       '.*'|\".*\"

FILEPATH            [0-9a-zA-Z_\-]+\.abh
QUOTED_FILEPATH     '[0-9a-zA-Z_\- ]+\.abh'|\"[0-9a-zA-Z_\- ]+\.abh\"

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
COMMON_TIME         [Cc]
VERSION_NUMBER      {DIGIT}\.{DIGIT}

INST_INCLUDE        abc-include
INST_CHARSET        abc-charset
INST_VERSION        abc-version
INST_CREATOR        abc-creator
INST_LINEBREAK      linebreak
INST_DECORATION     decoration

MACRO_TARGET        [~[:alnum:]]+

DECORATION          ![[:graph:]]+!|\+[[:graph:]]+\+
ANNOTATION          \"[^\"]+\"

REDEFINABLE_SYMBOL      [~h-wH-W]  
REDEFINABLE_REPLACEMENT {DECORATION}|{ANNOTATION}

NAME                (?i:name|nm)
SUBNAME             (?i:subname|snm)
VOICE_NAME          \"[^\"]+\"
STEM                (?i:stem)
UP                  (?i:up)
DOWN                (?i:down)

WSPACE              [[:blank:]]+

%%

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
<x_STR_INFO>.*      { yylval->s = NACStringTrimWhiteSpace(strdup(yytext)); return STRING; }

^K:                      { BEGIN(x_KEY); return KEY; }
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

^L:                            { BEGIN(x_UNIT_NOTE_LENGTH); return UNIT_NOTE_LENGTH; }
<x_UNIT_NOTE_LENGTH>{INTEGER}  { yylval->i = atoi(yytext); return INTEGER; }
<x_UNIT_NOTE_LENGTH>"/"        { return '/'; }


^M:                      { BEGIN(x_METER); return METER; }
<x_METER>{INTEGER}       { yylval->i = atoi(yytext); return INTEGER; }
<x_METER>{NONE}          { yylval->s = strdup(yytext); return NONE; }
<x_METER>{COMMON_TIME}   { return COMMON_TIME; }
<x_METER>"|"             { return '|'; }
<x_METER>"-"             { return '-'; }
<x_METER>"+"             { return '+'; }
<x_METER>"/"             { return '/'; }
<x_METER>"("             { return '('; }
<x_METER>")"             { return ')'; }

^Q:                      { BEGIN(x_TEMPO); return TEMPO; }
<x_TEMPO>{INTEGER}       { yylval->i = atoi(yytext); return INTEGER; }
<x_TEMPO>{FLOAT}         { yylval->i = atof(yytext); return FLOAT; }
<x_TEMPO>{QUOTED_STRING} { yylval->s = strdup(yytext + 1); yylval->s[strlen(yylval->s) - 1] = '\0'; return STRING; }
<x_TEMPO>"/"             { return '/'; }
<x_TEMPO>"="             { return '='; }
<x_TEMPO>"C"             { return 'C'; }

^P:                      { BEGIN(x_PARTS); return PARTS; }
<x_PARTS>[[:alpha:]]     { yylval->s = strdup(yytext); return PART_LABEL; }
<x_PARTS>{INTEGER}       { yylval->i = atoi(yytext); return INTEGER; }
<x_PARTS>"."             { }
<x_PARTS>"("             { return '('; }
<x_PARTS>")"             { return ')'; }

^I:                                { BEGIN(x_INSTRUCTION); return INSTRUCTION; }
<x_INSTRUCTION>{INST_INCLUDE}      { BEGIN(x_INST_INCLUDE); return INST_INCLUDE; }
<x_INST_INCLUDE>{FILEPATH}         { yylval->s = strdup(yytext); return FILEPATH; }
<x_INST_INCLUDE>{QUOTED_FILEPATH}  { yylval->s = strdup(yytext + 1); yylval->s[strlen(yylval->s) - 1] = '\0'; return FILEPATH; }

<x_INSTRUCTION>{INST_CHARSET}      { BEGIN(x_INST_CHARSET); return INST_CHARSET; }
<x_INST_CHARSET>.*                 { yylval->s = NACStringTrimWhiteSpace(strdup(yytext)); return STRING; }

<x_INSTRUCTION>{INST_VERSION}      { BEGIN(x_INST_VERSION); return INST_VERSION; }
<x_INST_VERSION>{VERSION_NUMBER}   { yylval->s = strdup(yytext); return VERSION_NUMBER; }

<x_INSTRUCTION>{INST_CREATOR}      { BEGIN(x_INST_CREATOR); return INST_CREATOR; }
<x_INST_CREATOR>.*                 { yylval->s = NACStringTrimWhiteSpace(strdup(yytext)); return STRING; }

<x_INSTRUCTION>{INST_LINEBREAK}    { BEGIN(x_INST_LINEBREAK); return INST_LINEBREAK; }
<x_INST_LINEBREAK>"<EOL>"          { yylval->s = strdup(yytext); return STRING; }
<x_INST_LINEBREAK>"<none>"         { yylval->s = strdup(yytext); return STRING; }
<x_INST_LINEBREAK>"!"              { yylval->c = yytext[0]; return CHAR; }
<x_INST_LINEBREAK>"$"              { yylval->c = yytext[0]; return CHAR; }

<x_INSTRUCTION>{INST_DECORATION}   { BEGIN(x_INST_DECORATION); return INST_DECORATION; }
<x_INST_DECORATION>"!"             { yylval->c = yytext[0]; return CHAR; }
<x_INST_DECORATION>"+"             { yylval->c = yytext[0]; return CHAR; }

m:                                 { BEGIN(x_MACRO); }
<x_MACRO>{MACRO_TARGET}            { yylval->s = strdup(yytext); return MACRO_TARGET; }
<x_MACRO>"="                       { BEGIN(x_MACRO_REPLACEMENT); return '='; }
<x_MACRO_REPLACEMENT>[^%]+         { yylval->s = NACStringTrimWhiteSpace(strdup(yytext)); return STRING; }

s:                  { BEGIN(x_SYMBOL_LINE); return SYMBOL_LINE; }
<x_SYMBOL_LINE>.*   { yylval->s = NACStringTrimWhiteSpace(strdup(yytext)); return STRING; }

U:                                              { BEGIN(x_REDEFINABLE_SYMBOL); }
<x_REDEFINABLE_SYMBOL>{REDEFINABLE_SYMBOL}      { yylval->s = strdup(yytext); return REDEFINABLE_SYMBOL; }
<x_REDEFINABLE_SYMBOL>"="                       { return '='; }
<x_REDEFINABLE_SYMBOL>{REDEFINABLE_REPLACEMENT} { yylval->s = strdup(yytext); return REDEFINABLE_REPLACEMENT; }

^X:                           { BEGIN(x_REFERENCE_NUMBER); return REFERENCE_NUMBER; }
<x_REFERENCE_NUMBER>{INTEGER} { yylval->i = atoi(yytext); return INTEGER; }

^r:                 { BEGIN(x_REMARK); }
<x_REMARK>.         { }

^\+:                { BEGIN(x_CONTINUATION); return CONTINUATION; }
<x_CONTINUATION>.*  { yylval->s = NACStringTrimWhiteSpace(strdup(yytext)); return STRING; }

^V:                          { BEGIN(x_VOICE); return VOICE; }
<x_VOICE>[[:alnum:]]+        { BEGIN(x_VOICE_PARAM); yylval->s = strdup(yytext); return VOICE_ID; }
<x_VOICE_PARAM>{NAME}        { return NAME; }
<x_VOICE_PARAM>{SUBNAME}     { return SUBNAME; }
<x_VOICE_PARAM>{VOICE_NAME}  { yylval->s = strdup(yytext); return VOICE_NAME; }
<x_VOICE_PARAM>{STEM}        { return STEM; }
<x_VOICE_PARAM>{UP}          { return UP; }
<x_VOICE_PARAM>{DOWN}        { return DOWN; }
<x_VOICE_PARAM>{INTEGER}     { yylval->i = atoi(yytext); return INTEGER; }
<x_VOICE_PARAM>{PITCH}       { yylval->s = strdup(yytext); return PITCH; }
<x_VOICE_PARAM>{CLEF}        { return CLEF; }
<x_VOICE_PARAM>{CLEF_NAME}   { yylval->s = strdup(yytext); return CLEF_NAME; }
<x_VOICE_PARAM>{MIDDLE}      { return MIDDLE; }
<x_VOICE_PARAM>{TRANSPOSE}   { return TRANSPOSE; }
<x_VOICE_PARAM>{OCTAVE}      { return OCTAVE; }
<x_VOICE_PARAM>{STAFF_LINES} { return STAFF_LINES; }
<x_VOICE_PARAM>"-"           { return '-'; }
<x_VOICE_PARAM>"+"           { return '+'; }
<x_VOICE_PARAM>"="           { return '='; }

<COMMENT>.          { }
<ERROR>.            { }

<*>"%"              {
                        switch (YY_START) {
                        case COMMENT:
                        case x_STR_INFO:
                        case x_REMARK:
                        case x_CONTINUATION:
                            break;
                        default:
                            BEGIN(COMMENT);
                            break;
                        }
                    }
<*>{WSPACE}         { }
<*>.                { return 1; }

<<EOF>>             { return 0; }

%%

void ABC_information_lex_set_error(yyscan_t scanner)
{
    struct yyguts_t *yyg = (struct yyguts_t *)scanner;
    BEGIN(ERROR);
}
