%{
#include <stdio.h>
struct Loc {
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} yylloc = {1,1,1,1};
void location(int cr);
%}

%x STRING
%x S_STRING
%x COMMENT
%x NOTE

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

VELOCITY       velocity
GATETIME       gatetime|gate_time
STEP           step
CHANNEL        channel
MSB            msb
LSB            lsb

IDENTIFER      [_[:alpha:]][[:alnum:]]*

%%
<<EOF>>          printf("END\n"); return 0;

{RESOLUTION}     location(0); printf("resolution\n");
{SET}            location(0); printf("set\n");
{UNSET}          location(0); printf("unset\n");
{DEFAULT}        location(0); printf("default\n");
{TEMPO}          location(0); printf("tempo\n");
{TRACK}          location(0); printf("track\n");
{TRACK_END}      location(0); printf("track_end|trackend\n");
{TIME_SIGNATURE} location(0); printf("time_signature|timesignature\n");
{BANK_SELECT}    location(0); printf("bank_select|bankselect\n");
{PROGRAM_CHANGE} location(0); printf("program_change|programchange\n");
{MARKER}         location(0); printf("marker\n");
{INCLUDE}        location(0); printf("include\n");
{NOTE}           location(0); printf("note\n");

{VELOCITY}       location(0); printf("velocity\n");
{GATETIME}       location(0); printf("gatetime|gate_time\n");
{STEP}           location(0); printf("step\n");
{CHANNEL}        location(0); printf("channel\n");
{MSB}            location(0); printf("msb\n");
{LSB}            location(0); printf("lsb\n");

{NOTE_NO}        location(0); printf("Note no: %s\n", yytext);

{M_B_TICK}       location(0); printf("Measure & beat & tick: %s\n", yytext);
{B_TICK}         location(0); printf("Beat & tick: %s\n", yytext);

{INTEGER}        location(0); printf("An integer: %s\n", yytext);
{FLOAT}          location(0); printf("A float: %s\n", yytext);

{WSPACE}         location(0);

"+"              location(0); printf("plus\n");
"-"              location(0); printf("plus\n");
"*"              location(0); printf("multiply\n");
"/"              location(0); printf("devision\n");
"="              location(0); printf("assign\n");
";"              location(0); printf("semicolon\n");
","              location(0); printf("Commma\n");

{IDENTIFER}      location(0); printf("Identifier: %s\n", yytext);

\n               location(1); printf("Return\n");

{LINE_COMMENT}     location(0); printf("Comment:\n%s\n", yytext);
"/*"               location(0); BEGIN(COMMENT); yymore();
<COMMENT>"*/"      location(0); printf("Comment:\n%s\n", yytext); BEGIN(INITIAL);
<COMMENT>\n        location(1); yymore();
<COMMENT>.         location(0); yymore();

\"                 location(0); BEGIN(STRING); yymore();
<STRING>\n         location(0); printf("Irregal literal: %s", yytext); BEGIN(INITIAL);
<STRING>[^\"\n]+   location(0); yymore();
<STRING>\"         location(0); printf("Literal string: %s\n", yytext); BEGIN(INITIAL);

\'                 location(0); BEGIN(S_STRING); yymore();
<S_STRING>\n       location(0); printf("Irregal literal: %s", yytext); BEGIN(INITIAL);
<S_STRING>[^\'\n]+ location(0); yymore();
<S_STRING>\'       location(0); printf("Literal string: %s\n", yytext); BEGIN(INITIAL);

.                  location(0); printf("Syntax Error. [%s] line=%d, column=%d\n", yytext, yylloc.last_line, yylloc.last_column);
%%

void location(int cr)
{
  yylloc.first_line   = yylloc.last_line;
  yylloc.first_column = yylloc.last_column + 1;
  if (cr) {
    ++yylloc.last_line;
    yylloc.last_column = 0;
  }
  else {
    yylloc.last_column += yyleng;
  }
}

int main(int argc, char** argv)
{
  if(argc > 1)
    yyin = fopen(argv[1], "r");
  int ret = yylex();
  printf("## %d lines\n", yylloc.last_line - 1);
  return ret;
}
