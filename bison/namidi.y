%{
#include <stdio.h>
#include <stdlib.h>
void yyerror(const char* s);
int yylex(void);

extern void location_init();
extern int last_line;
extern int last_column;

extern char *yytext;
%}

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

%token NOTE_NO

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

resolution: RESOLUTION expr { printf("resolution %d\n", $2); }
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
     | TRACK STRING
     | TRACK STRING assign_list
     | TRACK assign_list
     | TRACK assign_list STRING;

track_end: TRACK_END
         | TRACK_END assign_list;

time_signature: TIME_SIGNATURE INTEGER DIVISION INTEGER
              | TIME_SIGNATURE assign_list INTEGER DIVISION INTEGER
              | TIME_SIGNATURE INTEGER DIVISION INTEGER assign_list;

bank_select: BANK_SELECT assign_list;
program_change: PROGRAM_CHANGE INTEGER;

marker: MARKER STRING
      | MARKER STRING assign_list
      | MARKER assign_list STRING;

include: INCLUDE STRING;

note: NOTE
    | NOTE note_no_list
    | NOTE note_no_list assign_list
    | NOTE assign_list
    | NOTE assign_list note_no_list;

note_no_list: NOTE_NO
            | NOTE_NO COMMA NOTE_NO
            | note_no_list COMMA NOTE_NO;

expr: INTEGER                   { $$ = atoi(yytext); }
    | expr PLUS expr            { $$ = $1 + $3 }
    | expr MINUS expr           { $$ = $1 - $3 }
    | expr MULTIPLY expr        { $$ = $1 * $3 }
    | expr DIVISION expr        { $$ = $1 / $3 }
    | MINUS expr %prec NEGATIVE { $$ = -1 * $2 }
    | PLUS expr %prec POSITIVE  { $$ = $2 }
    ;

float_expr: FLOAT                     { $$ = atof(yytext); }
    | float_expr PLUS float_expr      { $$ = $1 + $3 }
    | float_expr MINUS float_expr     { $$ = $1 - $3 }
    | float_expr MULTIPLY float_expr  { $$ = $1 * $3 }
    | float_expr DIVISION float_expr  { $$ = $1 / $3 }
    | MINUS float_expr %prec NEGATIVE { $$ = -1 * $2 }
    | PLUS float_expr %prec POSITIVE  { $$ = $2 }
    ;

assign_list: assign
           | assign_list assign;

assign: VELOCITY ASSIGN expr { printf("velocity=%d\n", $3)}
      | GATETIME ASSIGN expr
      | STEP ASSIGN expr
      | STEP ASSIGN M_B_TICK
      | STEP ASSIGN B_TICK
      | CHANNEL ASSIGN expr
      | MSB ASSIGN expr
      | LSB ASSIGN expr;

param_list: param
          | param_list param;

param: VELOCITY
     | GATETIME { printf("param=%s\n", yytext); }
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
