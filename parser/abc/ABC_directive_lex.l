%{
  
#include "ABCParser.h"
#include "ABC_directive_yacc.h"
#include <string.h>
#include <NACString.h>

#define YY_USER_INIT \
    yycolumn = 1;

#define YY_USER_ACTION \
    yylloc->first_column = yycolumn; \
    if (strchr(yytext, '\n')) yycolumn = 1; \
    else yycolumn += yyleng;

%}

%option prefix="ABC_directive_"
%option outfile="ABC_directive_lex.c"
%option header-file="ABC_directive_lex.h"
%option reentrant
%option noyywrap
%option bison-bridge
%option bison-locations

%x ERROR
%x COMMENT

NONZERO                [1-9]
DIGIT                  [[:digit:]]
INTEGER                {NONZERO}{DIGIT}*|0

MIDI                   ^%%MIDI
VOICE                  (?i:voice)
INSTRUMENT             (?i:instrument)
BANK                   (?i:bank)
MUTE                   (?i:mute)
CHORDPROG              (?i:chordprog)
VOICE_ID               [[:alnum:]]+

PROPAGATE_ACCIDENTALS  ^%%propagate-accidentals
NOT                    (?i:not)
OCTAVE                 (?i:octave)
PITCH                  (?i:pitch)

UNSUPPORTED            ^%%[^Mp]

%%

{INTEGER}               { yylval->i = atoi(yytext); return INTEGER; }

{MIDI}                  { return MIDI; }
{VOICE}                 { return VOICE; }
{INSTRUMENT}            { return INSTRUMENT; }
{BANK}                  { return BANK; }
{MUTE}                  { return MUTE; }
{CHORDPROG}             { return CHORDPROG; }

{PROPAGATE_ACCIDENTALS} { return PROPAGATE_ACCIDENTALS; }
{NOT}                   { return NOT; }
{OCTAVE}                { return OCTAVE; }
{PITCH}                 { return PITCH; }

{VOICE_ID}              { yylval->s = strdup(yytext); return VOICE_ID; }

"="                     { return '='; }

"%"                     { BEGIN(COMMENT); }
{UNSUPPORTED}           { BEGIN(COMMENT); }

<COMMENT>.              { }
<ERROR>.                { }

[[:blank:]]             { }
.                       { return 1; }
<<EOF>>                 { return 0; }

%%

void ABC_directive_lex_set_error(yyscan_t scanner)
{
    struct yyguts_t *yyg = (struct yyguts_t *)scanner;
    BEGIN(ERROR);
}
