%{
#include <stdio.h>
int line_num = 1;
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

{RESOLUTION}     printf("resolution\n");
{SET}            printf("set\n");
{UNSET}          printf("unset\n");
{DEFAULT}        printf("default\n");
{TEMPO}          printf("tempo\n");
{TRACK}          printf("track\n");
{TRACK_END}      printf("track_end|trackend\n");
{TIME_SIGNATURE} printf("time_signature|timesignature\n");
{BANK_SELECT}    printf("bank_select|bankselect\n");
{PROGRAM_CHANGE} printf("program_change|programchange\n");
{MARKER}         printf("marker\n");
{INCLUDE}        printf("include\n");
{NOTE}           printf("note\n");

{VELOCITY}       printf("velocity\n");
{GATETIME}       printf("gatetime|gate_time\n");
{STEP}           printf("step\n");
{CHANNEL}        printf("channel\n");
{MSB}            printf("msb\n");
{LSB}            printf("lsb\n");

{NOTE_NO}        printf("Note no: %s\n", yytext);

{M_B_TICK}       printf("Measure & beat & tick: %s\n", yytext);
{B_TICK}         printf("Beat & tick: %s\n", yytext);

{INTEGER}        printf("An integer: %s\n", yytext);
{FLOAT}          printf("A float: %s\n", yytext);

{WSPACE}

"+"              printf("plus\n");
"-"              printf("plus\n");
"*"              printf("multiply\n");
"/"              printf("devision\n");
"="              printf("assign\n");
";"              printf("semicolon\n");
","              printf("Commma\n");

{IDENTIFER}      printf("Identifier: %s\n", yytext);

\n               printf("Return\n"); ++line_num;

{LINE_COMMENT}     printf("Comment:\n%s\n", yytext);
"/*"               BEGIN(COMMENT); yymore();
<COMMENT>"*/"      printf("Comment:\n%s\n", yytext); BEGIN(INITIAL);
<COMMENT>\n        yymore(); ++line_num;
<COMMENT>.         yymore();

\"                 BEGIN(STRING); yymore();
<STRING>\n         printf("Irregal literal: %s", yytext); BEGIN(INITIAL);
<STRING>[^\"\n]+   yymore();
<STRING>\"         printf("Literal string: %s\n", yytext); BEGIN(INITIAL);

\'                 BEGIN(S_STRING); yymore();
<S_STRING>\n       printf("Irregal literal: %s", yytext); BEGIN(INITIAL);
<S_STRING>[^\'\n]+ yymore();
<S_STRING>\'       printf("Literal string: %s\n", yytext); BEGIN(INITIAL);

.                  printf("Irregal character: %c\n", yytext[0]);
%%

int main(int argc, char** argv)
{
  if(argc > 1)
    yyin = fopen(argv[1], "r");
  int ret = yylex();
  printf("## %d lines\n", line_num);
  return ret;
}
