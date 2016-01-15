%{
  
#include "MML_yacc.h"
#include "MMLParser.h"
#include "NACString.h"
#include "NALog.h"

#include <string.h>

#define YY_USER_INIT \
    yylineno = 1; \
    yycolumn = 1;

#define YY_USER_ACTION \
    yylloc->first_line = yylineno; \
    yylloc->first_column = yycolumn; \
    if (strchr(yytext, '\n')) ++yylineno, yycolumn = 1; \
    else yycolumn += yyleng;

%}

%option prefix="MML_"
%option outfile="MML_lex.c"
%option header-file="MML_lex.h"
%option reentrant
%option noyywrap
%option bison-bridge
%option bison-locations
%option case-insensitive

%x COMMENT
%x x_INCLUDE

NONZERO        [1-9]
DIGIT          [[:digit:]]

INTEGER        {NONZERO}{DIGIT}*|0
FLOAT          ({NONZERO}{DIGIT}*|0)\.{DIGIT}+
STRING         \"[^\"]*\"|'[^']*'

D_TIMEBASE     ^[[:blank:]]*#[[:blank:]]*timebase
D_TITLE        ^[[:blank:]]*#[[:blank:]]*title
D_COPYRIGHT    ^[[:blank:]]*#[[:blank:]]*copyright
D_MARKER       ^[[:blank:]]*#[[:blank:]]*marker
D_VELOCITY     ^[[:blank:]]*#[[:blank:]]*velocity
D_OCTAVE       ^[[:blank:]]*#[[:blank:]]*octave
REVERSE        reverse

CHANNEL        @ch
SYNTHESIZER    @sy
BANK_SELECT    @bs
PROGRAM_CHANGE @pc
VOLUME         @vl
CHORUS         @cs
REVERB         @rv
EXPRESSION     @x
PAN            @p
DETUNE         @d
TEMPO          t

NOTE_LENGTH    ([[:digit:]]+\.{0,2}|%[[:digit:]]+)
NOTE           [CDEFGAB](\+{1,2}|#{1,2}|-{1,2})?{NOTE_LENGTH}?
REST           r{NOTE_LENGTH}?
OCTAVE         o
TRANSPOSE      @?ns
LENGTH         l
GATETIME       @?q
VELOCITY       @?v
TUPLET_START   \{
TUPLET_END     \}{NOTE_LENGTH}?
REPEAT_START   \/:
REPEAT_END     :\/
REPEAT_BREAK   \/

WSPACE         [[:blank:]]+
LINE_COMMENT   \/\/.*$

LOCATION       ^##.+:[[:digit:]]+:[[:digit:]]+\n

%%

{INTEGER}         { yylval->i = atoi(yytext); return INTEGER; }
{FLOAT}           { yylval->f = atof(yytext); return FLOAT; }
{STRING}          { yylval->s = strdup(yytext + 1);
                    yylval->s[yyleng - 2] = '\0';
                    return STRING; }

{D_TIMEBASE}      { return D_TIMEBASE; }
{D_TITLE}         { return D_TITLE; }
{D_COPYRIGHT}     { return D_COPYRIGHT; }
{D_MARKER}        { return D_MARKER; }
{D_VELOCITY}      { return D_VELOCITY; }
{D_OCTAVE}        { return D_OCTAVE; }
{REVERSE}         { return REVERSE; }
                         
{CHANNEL}         { return CHANNEL; }
{SYNTHESIZER}     { return SYNTHESIZER; }
{BANK_SELECT}     { return BANK_SELECT; }
{PROGRAM_CHANGE}  { return PROGRAM_CHANGE; }
{VOLUME}          { return VOLUME; }
{CHORUS}          { return CHORUS; }
{REVERB}          { return REVERB; }
{EXPRESSION}      { return EXPRESSION; }
{PAN}             { return PAN; }
{DETUNE}          { return DETUNE; }
{TEMPO}           { return TEMPO; }

{NOTE}            { yylval->s = strdup(yytext); return NOTE; }
{REST}            { yylval->s = strdup(yytext); return REST; }
{OCTAVE}          { return OCTAVE; }
{TRANSPOSE}       { return '@' == yytext[0] ? R_TRANSPOSE : TRANSPOSE; }
{LENGTH}          { return LENGTH; }
{GATETIME}        { return '@' == yytext[0] ? A_GATETIME : GATETIME; }
{VELOCITY}        { return '@' == yytext[0] ? A_VELOCITY : VELOCITY; }
{TUPLET_START}    { return TUPLET_START; }
{TUPLET_END}      { yylval->s = strdup(yytext + 1); return TUPLET_END; }
{REPEAT_START}    { return REPEAT_START; }
{REPEAT_END}      { return REPEAT_END; }
{REPEAT_BREAK}    { return REPEAT_BREAK; }
                         
","               { return ','; }
"-"               { return '-'; }
"+"               { return '+'; }
"<"               { return '<'; }
">"               { return '>'; }
"&"               { return '&'; }
"("               { return '('; }
")"               { return ')'; }
";"               { return ';'; }
"["               { return '['; }
"]"               { return ']'; }

{WSPACE}          { }
{LINE_COMMENT}    { }

"/*"              { BEGIN(COMMENT); }
<COMMENT>"*/"     { BEGIN(INITIAL); }
<COMMENT>.        { }

{LOCATION}        {
                      char *saveptr, *token, *s = NACStringDuplicate(yytext);
                      for (int i = 0; (token = strtok_r(s, "#:\n", &saveptr)); ++i) {
                          switch (i) {
                          case 0:
                              MMLParserSetCurrentFilepath(yyextra, token);
                              break;
                          case 1:
                              yylineno = atoi(token);
                              break;
                          case 2:
                              yycolumn = atoi(token);
                              break;
                          default:
                              break;
                          }
                          s = NULL;
                      }
                  }

.                 { return 1; }
<<EOF>>           { return 0; }
 
%%
