%{
 
#include "ABCParser.h"
#include "ABC_yacc.h"
#include "ABC_lex.h"
#include "ABCExpression.h"

extern int ABC_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, Expression **expression, const char *message);

#define PERSER() (ABC_get_extra(scanner))
#define LOC(yylloc) (&(ParseLocation){filepath, yylloc.first_line, yylloc.first_column})

%}

%name-prefix = "ABC_"
%output = "ABC_yacc.c"
%defines

%pure-parser
%lex-param   { yyscan_t scanner }
%parse-param { yyscan_t scanner }
%parse-param { const char *filepath }
%parse-param { Expression **expression }
%locations

%union {
    int i;
    float f;
    char *s;
    Expression *expression;
}

%token <s>NOTE

%token EOL

%type <expression> statement_list statement

%%
 
input
    :
    | statement_list                      { *expression = $1; }
    ;

statement_list
    : statement                           {
                                              if (!$1) YYABORT;

                                              if (ABCExprIsStatementList($1)) {
                                                  $$ = $1;
                                              }
                                              else {
                                                  $$ = ABCExprStatementList(PERSER(), LOC(@$));
                                                  ExpressionAddChild($$, $1);
                                              }
                                          }
    | statement_list statement            {
                                              if (!$2) YYABORT;

                                              if (ABCExprIsStatementList($2)) {
                                                  $$ = ABCExprStatementListMarge($1, $2);
                                              }
                                              else {
                                                  $$ = ExpressionAddChild($1, $2);
                                              }
                                          }

    ;

statement
    : EOL                                 { $$ = ABCExprEOL(PERSER(), LOC(@$)); }
    | NOTE                                { $$ = ABCExprNote(PERSER(), LOC(@$), $1); }
    ;

%%
