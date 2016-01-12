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

{INTEGER}         { ECHO; __Trace__ yylval->i = atoi(yytext); return INTEGER; }
{FLOAT}           { ECHO; __Trace__ yylval->f = atof(yytext); return FLOAT; }
{STRING}          { ECHO; __Trace__ yylval->s = strdup(yytext + 1);
                    yylval->s[yyleng - 2] = '\0';
                    return STRING; }

{D_TIMEBASE}      { ECHO; __Trace__ return D_TIMEBASE; }
{D_TITLE}         { ECHO; __Trace__ return D_TITLE; }
{D_COPYRIGHT}     { ECHO; __Trace__ return D_COPYRIGHT; }
{D_MARKER}        { ECHO; __Trace__ return D_MARKER; }
{D_VELOCITY}      { ECHO; __Trace__ return D_VELOCITY; }
{D_OCTAVE}        { ECHO; __Trace__ return D_OCTAVE; }
{REVERSE}         { ECHO; __Trace__ return REVERSE; }
                         
{CHANNEL}         { ECHO; __Trace__ return CHANNEL; }
{SYNTHESIZER}     { ECHO; __Trace__ return SYNTHESIZER; }
{BANK_SELECT}     { ECHO; __Trace__ return BANK_SELECT; }
{PROGRAM_CHANGE}  { ECHO; __Trace__ return PROGRAM_CHANGE; }
{VOLUME}          { ECHO; __Trace__ return VOLUME; }
{CHORUS}          { ECHO; __Trace__ return CHORUS; }
{REVERB}          { ECHO; __Trace__ return REVERB; }
{EXPRESSION}      { ECHO; __Trace__ return EXPRESSION; }
{PAN}             { ECHO; __Trace__ return PAN; }
{DETUNE}          { ECHO; __Trace__ return DETUNE; }
{TEMPO}           { ECHO; __Trace__ return TEMPO; }

{NOTE}            { ECHO; __Trace__ yylval->s = strdup(yytext); return NOTE; }
{REST}            { ECHO; __Trace__ yylval->s = strdup(yytext); return REST; }
{OCTAVE}          { ECHO; __Trace__ return OCTAVE; }
{TRANSPOSE}       { ECHO; __Trace__ return TRANSPOSE; }
{LENGTH}          { ECHO; __Trace__ return LENGTH; }
{GATETIME}        { ECHO; __Trace__ return GATETIME; }
{VELOCITY}        { ECHO; __Trace__ return VELOCITY; }
{TUPLET_START}    { ECHO; __Trace__ return TUPLET_START; }
{TUPLET_END}      { ECHO; __Trace__ yylval->s = strdup(yytext); return TUPLET_END; }
{REPEAT_START}    { ECHO; __Trace__ return REPEAT_START; }
{REPEAT_END}      { ECHO; __Trace__ return REPEAT_END; }
{REPEAT_BREAK}    { ECHO; __Trace__ return REPEAT_BREAK; }
                         
","               { ECHO; __Trace__ return ','; }
"-"               { ECHO; __Trace__ return '-'; }
"+"               { ECHO; __Trace__ return '+'; }
"<"               { ECHO; __Trace__ return '<'; }
">"               { ECHO; __Trace__ return '>'; }
"&"               { ECHO; __Trace__ return '&'; }
"("               { ECHO; __Trace__ return '('; }
")"               { ECHO; __Trace__ return ')'; }
";"               { ECHO; __Trace__ return ';'; }
"["               { ECHO; __Trace__ return '['; }
"]"               { ECHO; __Trace__ return ']'; }

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
