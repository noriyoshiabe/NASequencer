%{
 
#include "ABCParser.h"
#include "ABC_yacc.h"
#include "ABC_lex.h"
#include "ABCExpression.h"
#include "NAArray.h"

extern int ABC_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, Expression **expression, const char *message);

#define PERSER() (ABC_get_extra(scanner))
#define LOC(yylloc) (&(ParseLocation){(char *)filepath, yylloc.first_line, yylloc.first_column})
#define SKIP(yylloc) ExpressionCreateSkip(LOC(yylloc))

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
    NAArray *array;
}

%token EOL
%token <s>VERSION
%token <s>REFERENCE_NUMBER
%token <s>TUNE_TITLE
%token KEY
%token <s>KEY_TONIC
%token <s>KEY_MODE
%token <s>KEY_ACCIDENTAL
%token <s>NOTE

%type <expression> statement_list statement
%type <expression> key_expr_list key_expr
%type <array>      key_accidental_list

%%
 
input
    : statement_list                      { *expression = $1; }
    ;

statement_list
    :                                     { $$ = ABCExprStatementList(PERSER(), LOC(@$)); }
    | statement                           { if (!$1) $1 = SKIP(@$); $$ = ExpressionAddChild(ABCExprStatementList(PERSER(), LOC(@$)), $1); }
    | statement_list statement            { if (!$2) $2 = SKIP(@$); $$ = ExpressionAddChild($1, $2); }
    ;

statement
    : EOL                                 { $$ = ABCExprEOL(PERSER(), LOC(@$)); }
    | VERSION                             { $$ = ABCExprVersion(PERSER(), LOC(@$), $1); }
    | REFERENCE_NUMBER                    { $$ = ABCExprReferenceNumber(PERSER(), LOC(@$), $1); }
    | KEY key_expr_list                   { $$ = $2; }
    | TUNE_TITLE                          { $$ = ABCExprTuneTitle(PERSER(), LOC(@$), $1); }
    | NOTE                                { $$ = ABCExprNote(PERSER(), LOC(@$), $1); }

    | error                               { $$ = SKIP(@$); }
    ;

key_expr_list
    :                                     { $$ = ABCExprKeyExprList(PERSER(), LOC(@$)); }
    | key_expr                            { if (!$1) $1 = SKIP(@$); $$ = ExpressionAddChild(ABCExprKeyExprList(PERSER(), LOC(@$)), $1); }
    | key_expr_list key_expr              { if (!$2) $2 = SKIP(@$); $$ = ExpressionAddChild($1, $2); }
    ;

key_expr
    : KEY_TONIC                           { $$ = ABCExprKeySign(PERSER(), LOC(@$), $1, NULL, NULL); }
    | KEY_TONIC KEY_MODE                  { $$ = ABCExprKeySign(PERSER(), LOC(@$), $1, $2, NULL); }
    | KEY_TONIC KEY_MODE key_accidental_list
                                          { $$ = ABCExprKeySign(PERSER(), LOC(@$), $1, $2, $3); }
    | KEY_TONIC key_accidental_list       { $$ = ABCExprKeySign(PERSER(), LOC(@$), $1, NULL, $2); }

key_accidental_list
    : KEY_ACCIDENTAL                      { $$ = NAArrayCreate(4, NADescriptionCString); NAArrayAppend($$, $1); }
    | key_accidental_list KEY_ACCIDENTAL  { $$ = $1; NAArrayAppend($$, $2); }
    ;

%%
