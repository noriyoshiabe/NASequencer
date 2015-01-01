%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void yyerror(const char* s);
int yylex(void);

extern void location_init();
extern int last_line;
extern int last_column;

extern char *yytext;

static char *trim_literal(char *str);
static char *trim_last(char *str);

extern int interpret(int action, int modifier, void *arg);
#define INTERPRET_STATEMENT(modifier) \
  do { \
    if (interpret(STATEMENT, modifier, NULL)) { \
      YYERROR; \
    } \
  } while (0)
#define INTERPRET_ASSIGN(modifier, type, arg) \
  do { \
    type _arg; \
    _arg = (type)arg; \
    if (interpret(ASSIGN, modifier, (void *)&_arg)) { \
      YYERROR; \
    } \
  } while (0)
%}

%union {
  float f;
  char *s;
}

%token RESOLUTION
%token SET
%token UNSET
%token DEFAULT
%token TEMPO
%token TRACK
%token TRACK_END
%token TIME_SIGNATURE
%token BANK_SELECT
%token PROGRAM_CHANGE
%token MARKER
%token INCLUDE
%token NOTE

%token VELOCITY
%token GATETIME
%token STEP
%token CHANNEL
%token MSB
%token LSB

%token NOTE_NO_LIST

%token M_B_TICK
%token B_TICK

%token INTEGER
%token FLOAT
%token STRING

%token WSPACE

%token PLUS
%token MINUS
%token MULTIPLY
%token DIVISION
%token ASSIGN
%token COMMA

%token IDENTIFIER

%token EOS
%token EOL

%left PLUS MINUS
%left MULTIPLY DIVISION
%left NEGATIVE POSITIVE
%left ASSIGN

%token NAME
%token NUMERATOR
%token DENOMINATOR

%token STATEMENT

%%

input:
     | input line;

line: EOL
    | statement EOL
    | statement EOS
    | line EOL;

statement: resolution     { INTERPRET_STATEMENT(RESOLUTION); }
         | set            { INTERPRET_STATEMENT(SET); }
         | unset          { INTERPRET_STATEMENT(UNSET); }
         | tempo          { INTERPRET_STATEMENT(TEMPO); }
         | track          { INTERPRET_STATEMENT(TRACK); }
         | track_end      { INTERPRET_STATEMENT(TRACK_END); }
         | time_signature { INTERPRET_STATEMENT(TIME_SIGNATURE); }
         | bank_select    { INTERPRET_STATEMENT(BANK_SELECT); }
         | program_change { INTERPRET_STATEMENT(PROGRAM_CHANGE); }
         | marker         { INTERPRET_STATEMENT(MARKER); }
         | include        { INTERPRET_STATEMENT(INCLUDE); }
         | note           { INTERPRET_STATEMENT(NOTE); };

resolution: RESOLUTION assign_resolution;

set: SET assign_list
   | SET assign_default assign_list;

unset: UNSET unassign_list
     | UNSET assign_default unassign_list;

tempo: TEMPO assign_tempo
     | TEMPO assign_tempo assign_list
     | TEMPO assign_list assign_tempo
     ;

track: TRACK            
     | TRACK assign_name
     | TRACK assign_name assign_list
     | TRACK assign_list
     | TRACK assign_list assign_name;

track_end: TRACK_END
         | TRACK_END assign_list;

time_signature: TIME_SIGNATURE assign_time_signature
              | TIME_SIGNATURE assign_list assign_time_signature
              | TIME_SIGNATURE assign_time_signature assign_list;

bank_select: BANK_SELECT assign_list;
program_change: PROGRAM_CHANGE assign_program_change;
marker: MARKER assign_name
      | MARKER assign_name assign_list
      | MARKER assign_list assign_name;

include: INCLUDE assign_name;

note: NOTE
    | NOTE assign_note_no_list
    | NOTE assign_note_no_list assign_list
    | NOTE assign_list
    | NOTE assign_list assign_note_no_list;

expr: INTEGER                   { $<f>$ = atof(yytext); }
    | FLOAT                     { $<f>$ = atof(yytext); }
    | expr PLUS expr            { $<f>$ = $<f>1 + $<f>3 }
    | expr MINUS expr           { $<f>$ = $<f>1 - $<f>3 }
    | expr MULTIPLY expr        { $<f>$ = $<f>1 * $<f>3 }
    | expr DIVISION expr        { $<f>$ = $<f>1 / $<f>3 }
    | MINUS expr %prec NEGATIVE { $<f>$ = -1.0 * $<f>2 }
    | PLUS expr %prec POSITIVE  { $<f>$ = $<f>2 };

string_expr:       STRING       { $<s>$ = trim_literal(yytext); };
note_no_list_expr: NOTE_NO_LIST { $<s>$ = trim_last(yytext); };
mb_tick_expr:      M_B_TICK     { $<s>$ = trim_last(yytext); };
b_tick_expr :      B_TICK       { $<s>$ = trim_last(yytext); };

assign_list: assign
           | assign_list assign;

assign: VELOCITY ASSIGN expr     { INTERPRET_ASSIGN(VELOCITY, int, $<f>3) }
      | GATETIME ASSIGN expr     { INTERPRET_ASSIGN(GATETIME, int, $<f>3) }
      | STEP ASSIGN expr         { INTERPRET_ASSIGN(STEP, int, $<f>3) }
      | STEP ASSIGN mb_tick_expr { INTERPRET_ASSIGN(M_B_TICK, char *, $<s>3) }
      | STEP ASSIGN b_tick_expr  { INTERPRET_ASSIGN(B_TICK, char *, $<s>3) }
      | CHANNEL ASSIGN expr      { INTERPRET_ASSIGN(CHANNEL, int, $<f>3) }
      | MSB ASSIGN expr          { INTERPRET_ASSIGN(MSB, int, $<f>3) }
      | LSB ASSIGN expr          { INTERPRET_ASSIGN(LSB, int, $<f>3) };

assign_resolution:     expr        { INTERPRET_ASSIGN(RESOLUTION, int, $<f>1) };
assign_default:        DEFAULT     { INTERPRET_ASSIGN(DEFAULT, int, 1) };
assign_tempo:          expr        { INTERPRET_ASSIGN(TEMPO, float, $<f>1) };
assign_name:           string_expr { INTERPRET_ASSIGN(NAME, char *, $<s>1) };
assign_time_signature: expr DIVISION expr {
                                     INTERPRET_ASSIGN(NUMERATOR, int, $<f>1);
                                     INTERPRET_ASSIGN(DENOMINATOR, int, $<f>2);};
assign_program_change: expr { INTERPRET_ASSIGN(PROGRAM_CHANGE, int, $<f>1) };
assign_note_no_list:  note_no_list_expr { INTERPRET_ASSIGN(NOTE_NO_LIST, char *, $<s>1) };

unassign_list: unassign
             | unassign_list unassign;

unassign: VELOCITY { INTERPRET_ASSIGN(VELOCITY, int, -1); }
        | GATETIME { INTERPRET_ASSIGN(GATETIME, int, -1); }
        | STEP     { INTERPRET_ASSIGN(STEP, int, -1); }
        | CHANNEL  { INTERPRET_ASSIGN(CHANNEL, int, -1); }
        | MSB      { INTERPRET_ASSIGN(MSB, int, -1); }
        | LSB      { INTERPRET_ASSIGN(LSB, int, -1); };

%%

int main(void)
{
  location_init();
  return yyparse();
}

void yyerror(const char* s)
{
  fprintf(stderr, "Error: %s line=%d column=%d\n", s, last_line, last_column);
}

char *trim_literal(char *str)
{
  if ('\'' == str[0] || '"' == str[0]) {
    *strrchr(str, str[0]) = '\0';
    return str + 1;
  } else {
    return str;
  }
}

char *trim_last(char *str)
{
  char *p;
  while ((p = strrchr(str, '\n')) || (p = strrchr(str, '\r'))) {
    *p = '\0';
  }
  return str;
}

#define TO_S(sym) #sym

int interpret(int action, int modifier, void *arg)
{
  switch (action) {
  case STATEMENT:
    printf("%s ", TO_S(STATEMENT));
    switch(modifier) {
    case RESOLUTION: printf("%s\n", TO_S(RESOLUTION)); break;
    case SET: printf("%s\n", TO_S(SET)); break;
    case UNSET: printf("%s\n", TO_S(UNSET)); break;
    case TEMPO: printf("%s\n", TO_S(TEMPO)); break;
    case TRACK: printf("%s\n", TO_S(TRACK)); break;
    case TRACK_END: printf("%s\n", TO_S(TRACK_END)); break;
    case TIME_SIGNATURE: printf("%s\n", TO_S(TIME_SIGNATURE)); break;
    case BANK_SELECT: printf("%s\n", TO_S(BANK_SELECT)); break;
    case PROGRAM_CHANGE: printf("%s\n", TO_S(PROGRAM_CHANGE)); break;
    case MARKER: printf("%s\n", TO_S(MARKER)); break;
    case INCLUDE: printf("%s\n", TO_S(INCLUDE)); break;
    case NOTE: printf("%s\n", TO_S(NOTE)); break;
    }
    break;

  case ASSIGN:
    printf("%s ", TO_S(ASSIGN));
    switch (modifier) {
    case RESOLUTION: printf("%s=%d\n", TO_S(RESOLUTION), *((int *)arg)); break;
    case DEFAULT: printf("%s=%d\n", TO_S(DEFAULT), *((int *)arg)); break;
    case TEMPO: printf("%s=%f\n", TO_S(TEMPO), *((float *)arg)); break;
    case NUMERATOR: printf("%s=%d\n", TO_S(NUMERATOR), *((int *)arg)); break;
    case DENOMINATOR: printf("%s=%d\n", TO_S(DENOMINATOR), *((int *)arg)); break;
    case PROGRAM_CHANGE: printf("%s=%d\n", TO_S(PROGRAM_CHANGE), *((int *)arg)); break;
    case VELOCITY: printf("%s=%d\n", TO_S(VELOCITY), *((int *)arg)); break;
    case GATETIME: printf("%s=%d\n", TO_S(GATETIME), *((int *)arg)); break;
    case STEP: printf("%s=%d\n", TO_S(STEP), *((int *)arg)); break;
    case CHANNEL: printf("%s=%d\n", TO_S(CHANNEL), *((int *)arg)); break;
    case MSB: printf("%s=%d\n", TO_S(MSB), *((int *)arg)); break;
    case LSB: printf("%s=%d\n", TO_S(LSB), *((int *)arg)); break;
    
    case M_B_TICK: printf("%s=%s\n", TO_S(M_B_TICK), *((char **)arg)); break;
    case B_TICK: printf("%s=%s\n", TO_S(B_TICK), *((char **)arg)); break;
    case NAME: printf("%s=%s\n", TO_S(NAME), *((char **)arg)); break;
    case NOTE_NO_LIST: printf("%s=%s\n", TO_S(NOTE_NO_LIST), *((char **)arg)); break;
    }
    break;
  }

  return 0;
}
