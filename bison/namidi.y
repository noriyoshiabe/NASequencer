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
%}

%union {
  int i;
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

%token <s>NOTE_NO_LIST

%token <s>M_B_TICK
%token <s>B_TICK

%token <i>INTEGER
%token <f>FLOAT
%token <s>STRING

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

%%

input:
     | input line;

line: EOL
    | statement EOL
    | statement EOS
    | line EOL;

statement: resolution
         | set
         | unset
         | tempo
         | track
         | track_end
         | time_signature
         | bank_select
         | program_change
         | marker
         | include
         | note;

resolution: RESOLUTION float_expr { printf("resolution %f\n", $<f>2); }
          ;

set: SET assign_list
   | SET DEFAULT assign_list { printf("set default\n"); }
   ;

unset: UNSET param_list
     | UNSET DEFAULT param_list { printf("unset default\n"); }
     ;

tempo: TEMPO float_expr
     | TEMPO float_expr assign_list
     | TEMPO assign_list float_expr;

track: TRACK
     | TRACK string_expr { printf("[%s]\n", $<s>2); }
     | TRACK string_expr assign_list
     | TRACK assign_list
     | TRACK assign_list string_expr;

track_end: TRACK_END
         | TRACK_END assign_list;

time_signature: TIME_SIGNATURE INTEGER DIVISION INTEGER
              | TIME_SIGNATURE assign_list INTEGER DIVISION INTEGER
              | TIME_SIGNATURE INTEGER DIVISION INTEGER assign_list;

bank_select: BANK_SELECT assign_list;
program_change: PROGRAM_CHANGE INTEGER;

marker: MARKER string_expr
      | MARKER string_expr assign_list
      | MARKER assign_list string_expr;

include: INCLUDE string_expr;

note: NOTE
    | NOTE note_no_list_expr { printf("--- [%s]\n", trim_last($<s>2)); }
    | NOTE note_no_list_expr assign_list { printf("--- [%s]\n", trim_last($<s>2)); }
    | NOTE assign_list
    | NOTE assign_list note_no_list_expr;

expr: INTEGER                   { $<i>$ = atoi(yytext); }
    | expr PLUS expr            { $<i>$ = $<i>1 + $<i>3 }
    | expr MINUS expr           { $<i>$ = $<i>1 - $<i>3 }
    | expr MULTIPLY expr        { $<i>$ = $<i>1 * $<i>3 }
    | expr DIVISION expr        { $<i>$ = $<i>1 / $<i>3 }
    | MINUS expr %prec NEGATIVE { $<i>$ = -1 * $<i>2 }
    | PLUS expr %prec POSITIVE  { $<i>$ = $<i>2 }
    ;

float_expr: FLOAT                           { $<f>$ = atof(yytext); }
          | INTEGER                         { $<f>$ = atof(yytext); }
          | float_expr PLUS float_expr      { $<f>$ = $<f>1 + $<f>3 }
          | float_expr MINUS float_expr     { $<f>$ = $<f>1 - $<f>3 }
          | float_expr MULTIPLY float_expr  { $<f>$ = $<f>1 * $<f>3 }
          | float_expr DIVISION float_expr  { $<f>$ = $<f>1 / $<f>3 }
          | MINUS float_expr %prec NEGATIVE { $<f>$ = -1.0 * $<f>2 }
          | PLUS float_expr %prec POSITIVE  { $<f>$ = $<f>2 }
          ;

string_expr: STRING { $<s>$ = trim_literal(yytext); }

note_no_list_expr: NOTE_NO_LIST { $<s>$ = trim_last(yytext); }

mb_tick_expr: M_B_TICK  { $<s>$ = trim_last(yytext); }
            | B_TICK    { $<s>$ = trim_last(yytext); }

assign_list: assign
           | assign_list assign;

assign: VELOCITY ASSIGN expr { printf("velocity=%d\n", $<i>3)}
      | GATETIME ASSIGN expr
      | STEP ASSIGN expr
      | STEP ASSIGN mb_tick_expr { printf("step=[%s]\n", $<s>3)}
      | CHANNEL ASSIGN expr
      | MSB ASSIGN expr
      | LSB ASSIGN expr;

param_list: param
          | param_list param;

param: VELOCITY
     | GATETIME { printf("param=[%s]\n", yytext); }
     | STEP
     | CHANNEL
     | MSB
     | LSB;

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
