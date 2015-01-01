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

resolution: RESOLUTION expr { printf("resolution %d\n", (int)$<f>2); }
          ;

set: SET assign_list
   | SET DEFAULT assign_list { printf("set default\n"); }
   ;

unset: UNSET param_list
     | UNSET DEFAULT param_list { printf("unset default\n"); }
     ;

tempo: TEMPO expr { printf("tempo=%f\n", $<f>2); }
     | TEMPO expr assign_list
     | TEMPO assign_list expr;

track: TRACK
     | TRACK string_expr { printf("[%s]\n", $<s>2); }
     | TRACK string_expr assign_list
     | TRACK assign_list
     | TRACK assign_list string_expr;

track_end: TRACK_END
         | TRACK_END assign_list;

time_signature: TIME_SIGNATURE expr DIVISION expr    { printf("timesig %d/%d\n", (int)$<f>2, (int)$<f>4); }
              | TIME_SIGNATURE assign_list expr DIVISION expr
              | TIME_SIGNATURE expr DIVISION expr assign_list;

bank_select: BANK_SELECT assign_list;
program_change: PROGRAM_CHANGE expr { printf("program change [%d]\n", (int)$<f>2); };

marker: MARKER string_expr
      | MARKER string_expr assign_list
      | MARKER assign_list string_expr;

include: INCLUDE string_expr;

note: NOTE
    | NOTE note_no_list_expr { printf("--- [%s]\n", trim_last($<s>2)); }
    | NOTE note_no_list_expr assign_list { printf("--- [%s]\n", trim_last($<s>2)); }
    | NOTE assign_list
    | NOTE assign_list note_no_list_expr;

expr: INTEGER                   { $<f>$ = atof(yytext); }
    | FLOAT                     { $<f>$ = atof(yytext); }
    | expr PLUS expr            { $<f>$ = $<f>1 + $<f>3 }
    | expr MINUS expr           { $<f>$ = $<f>1 - $<f>3 }
    | expr MULTIPLY expr        { $<f>$ = $<f>1 * $<f>3 }
    | expr DIVISION expr        { $<f>$ = $<f>1 / $<f>3 }
    | MINUS expr %prec NEGATIVE { $<f>$ = -1.0 * $<f>2 }
    | PLUS expr %prec POSITIVE  { $<f>$ = $<f>2 }
    ;

string_expr: STRING { $<s>$ = trim_literal(yytext); }

note_no_list_expr: NOTE_NO_LIST { $<s>$ = trim_last(yytext); }

mb_tick_expr: M_B_TICK  { $<s>$ = trim_last(yytext); }
            | B_TICK    { $<s>$ = trim_last(yytext); }

assign_list: assign
           | assign_list assign;

assign: VELOCITY ASSIGN expr { printf("velocity=%d\n", (int)$<f>3)}
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
