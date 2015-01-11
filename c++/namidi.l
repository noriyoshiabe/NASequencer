%{
#include <stdio.h>
#include "namidi.tab.h"
void location(int cr);
int yywrap() { return 1; }
%}

%x COMMENT

NONZERO        [1-9]
DIGIT          [[:digit:]]
INTEGER        {NONZERO}{DIGIT}*|0
FLOAT          ({NONZERO}{DIGIT}*\.?|0\.|\.{DIGIT}){DIGIT}*|0
B_TICK         {NONZERO}{DIGIT}?:{DIGIT}{1,3}
M_B_TICK       {NONZERO}{DIGIT}{0,2}:{NONZERO}{DIGIT}?:{DIGIT}{1,3}
LINE_COMMENT   \/\/.*$
WSPACE         [[:blank:]]+

KEY            c|d|e|f|g|a|b
NOTE_NO        {KEY}[#]?({DIGIT}|[+-]{DIGIT})?
NOTE_NO_LIST   {NOTE_NO}[[:blank:]]*(,[[:blank:]]*{NOTE_NO})*

RESOLUTION     resolution
SET            set
UNSET          unset
DEFAULT        default
TEMPO          tempo
TRACK          track
TRACK_END      track_end|trackend
TIME_SIGNATURE time_signature|timesignature
BANK_SELECT    bank_select|bankselect
PROGRAM_CHANGE program_change|programchange
MARKER         marker
INCLUDE        include
NOTE           note
REST           rest
TIE            tie

VELOCITY       velocity
GATETIME       gatetime|gate_time
STEP           step
CHANNEL        channel
MSB            msb
LSB            lsb

IDENTIFIER     [_[:alpha:]][[:alnum:]]*

%%
<<EOF>>          return 0;

{RESOLUTION}     location(0); return RESOLUTION;
{SET}            location(0); return SET;
{UNSET}          location(0); return UNSET;
{DEFAULT}        location(0); return DEFAULT;
{TEMPO}          location(0); return TEMPO;
{TRACK}          location(0); return TRACK;
{TRACK_END}      location(0); return TRACK_END;
{TIME_SIGNATURE} location(0); return TIME_SIGNATURE;
{BANK_SELECT}    location(0); return BANK_SELECT;
{PROGRAM_CHANGE} location(0); return PROGRAM_CHANGE;
{MARKER}         location(0); return MARKER;
{INCLUDE}        location(0); return INCLUDE;
{NOTE}           location(0); return NOTE;
{REST}           location(0); return REST;
{TIE}            location(0); return TIE;

{VELOCITY}       location(0); return VELOCITY;
{GATETIME}       location(0); return GATETIME;
{STEP}           location(0); return STEP;
{CHANNEL}        location(0); return CHANNEL;
{MSB}            location(0); return MSB;
{LSB}            location(0); return LSB;

{NOTE_NO_LIST}   location(0); return NOTE_NO_LIST;

{M_B_TICK}       location(0); return M_B_TICK;
{B_TICK}         location(0); return B_TICK;

{INTEGER}        location(0); return INTEGER;
{FLOAT}          location(0); return FLOAT;

{WSPACE}         location(0);

"+"              location(0); return PLUS;
"-"              location(0); return MINUS;
"*"              location(0); return MULTIPLY;
"/"              location(0); return DIVISION;
"="              location(0); return ASSIGN;
";"              location(0); return EOS;
","              location(0); return COMMA;

\"[^\"]*\"       location(0); return STRING;
'[^']*'          location(0); return STRING;

{IDENTIFIER}     location(0); return IDENTIFIER;

\n               location(1); return EOL;

{LINE_COMMENT}     location(0);
"/*"               location(0); BEGIN(COMMENT); yymore();
<COMMENT>"*/"      location(0); BEGIN(INITIAL);
<COMMENT>\n        location(1); yymore();
<COMMENT>.         location(0); yymore();

%%

int last_line;
int last_column;
int current_line;
int current_column;

void location_init()
{
  current_line = last_line = 1;
  current_column = last_column = 0;
}

void location(int cr)
{
  last_line = current_line;
  last_column = current_column;
  if (cr) {
    ++current_line;
    current_column = 0;
  }
  else {
    current_column += yyleng;
  }
}
