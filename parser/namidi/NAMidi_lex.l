%{
  
#include "NAMidi_yacc.h"
#include "NAMidiParser.h"
#include "NACString.h"

#include <string.h>

#define YY_USER_INIT \
    yylineno = 1; \
    yycolumn = 1;

#define YY_USER_ACTION \
    yylloc->first_line = yylineno; \
    yylloc->first_column = yycolumn; \
    if (strchr(yytext, '\n')) ++yylineno, yycolumn = 1; \
    else yycolumn += yyleng;

#define ECHO

%}

%option prefix="NAMidi_"
%option outfile="NAMidi_lex.c"
%option header-file="NAMidi_lex.h"
%option reentrant
%option noyywrap
%option bison-bridge
%option bison-locations
%option case-insensitive

%x x_INCLUDE

U              [\x80-\xbf]
U2             [\xc2-\xdf]
U3             [\xe0-\xef]
U4             [\xf0-\xf4]
UNICORD        {U2}{U}|{U3}{U}{U}|{U4}{U}{U}{U}

NONZERO        [1-9]
DIGIT          [[:digit:]]

INTEGER        {NONZERO}{DIGIT}*|0
FLOAT          ({NONZERO}{DIGIT}*|0)\.{DIGIT}+
STRING         \"[^\"]*\"|'[^']*'

RESOLUTION     resolution
TITLE          title
COPYRIGHT      copyright
TEMPO          tempo
TIME           time
MARKER         marker
PATTERN        pattern
EXPAND         expand
WITH           with
END            end
CHANNEL        channel
VELOCITY       velocity
GATETIME       gatetime
STEP           step
PROGRAM        program
BANK           bank
SYNTH          synth
VOLUME         volume
PAN            pan
CHORUS         chorus
REVERB         reverb
EXPRESSION     expression
DETUNE         detune
TRANSPOSE      transpose
KEY            key
PERCUSSION     percussion
PITCH          pitch
SENSE          sense
SUSTAIN        sustain

KEY_SIGN       [cdefgab](#|b)?[[:blank:]]*(maj(or)?|min(or)?)
NOTE           [cdefgab](#|##|b|bb|n)?(\-?[[:digit:]]{1,2})?
NOTE_PARAM_V   v
NOTE_PARAM_GT  gt
ON             on
OFF            off
ID_STRING      [_[:alnum:]][_[:alnum:]\-]*
IDENTIFIER     {ID_STRING}(:{ID_STRING})*

WSPACE         [[:blank:]]+

LOCATION       ^##.+:[[:digit:]]+:[[:digit:]]+\n

%%

{INTEGER}        { yylval->i = atoi(yytext); return INTEGER; }
{FLOAT}          { yylval->f = atof(yytext); return FLOAT; }
{STRING}         { yylval->s = strdup(yytext + 1);
                   yylval->s[strlen(yylval->s) - 1] = '\0';
                   return STRING; }

{RESOLUTION}     { return RESOLUTION; }
{TITLE}          { return TITLE; }
{COPYRIGHT}      { return COPYRIGHT; }
{TEMPO}          { return TEMPO; }
{TIME}           { return TIME; }
{KEY}            { return KEY; }
{PERCUSSION}     { return PERCUSSION; }
{MARKER}         { return MARKER; }
{PATTERN}        { return PATTERN; }
{EXPAND}         { return EXPAND; }
{END}            { return END; }
{CHANNEL}        { return CHANNEL; }
{VELOCITY}       { return VELOCITY; }
{GATETIME}       { return GATETIME; }
{STEP}           { return STEP; }
{PROGRAM}        { return PROGRAM; }
{BANK}           { return BANK; }
{SYNTH}          { return SYNTH; }
{VOLUME}         { return VOLUME; }
{PAN}            { return PAN; }
{CHORUS}         { return CHORUS; }
{REVERB}         { return REVERB; }
{EXPRESSION}     { return EXPRESSION; }
{DETUNE}         { return DETUNE; }
{TRANSPOSE}      { return TRANSPOSE; }
{PITCH}          { return PITCH; }
{SENSE}          { return SENSE; }
{SUSTAIN}        { return SUSTAIN; }

{NOTE}           { yylval->s = strdup(yytext); return NOTE; }
{NOTE_PARAM_V}   { return NOTE_PARAM_V; }
{NOTE_PARAM_GT}  { return NOTE_PARAM_GT; }
{ON}             { return ON; }
{OFF}            { return OFF; }
{KEY_SIGN}       { yylval->s = strdup(yytext); return KEY_SIGN; }
{IDENTIFIER}     { yylval->s = strdup(yytext); return IDENTIFIER; }

"="              { return '='; }
"+"              { return '+'; }
"-"              { return '-'; }
"/"              { return '/'; }
";"              { return ';'; }
":"              { return ':'; }
\n               { return '\n'; }

{WSPACE}         { }

{LOCATION}       {
                     char *saveptr, *token, *s = NACStringDuplicate(yytext);
                     for (int i = 0; (token = strtok_r(s, "#:\n", &saveptr)); ++i) {
                         switch (i) {
                         case 0:
                             NAMidiParserSetCurrentFilepath(yyextra, token);
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

{UNICORD}        { return 1; }
.                { return 1; }
<<EOF>>          { return 0; }
 
%%
