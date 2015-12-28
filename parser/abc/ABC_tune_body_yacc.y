%{
 
#include "ABC_tune_body_yacc.h"
#include "ABC_tune_body_lex.h"

#include "ABCParser.h"
#include "ABCAST.h"

extern int ABC_tune_body_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, int line, void **node, const char *message);

#define node(type, yylloc) ABCAST##type##Create(&((FileLocation){(char *)filepath, line, yylloc.first_column}))
#define list() NAArrayCreate(4, NULL)
#define listAppend(list, node) NAArrayAppend(list, node)

#if 1
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

%}

%name-prefix = "ABC_tune_body_"
%output = "ABC_tune_body_yacc.c"
%defines

%pure-parser
%lex-param   { yyscan_t scanner }
%parse-param { yyscan_t scanner }
%parse-param { const char *filepath }
%parse-param { int line }
%parse-param { void **node }
%locations

%union {
    char *s;
    void *node;
    void *list;
}

%token <s>INLINE_FIELD

%type <node> inline_field line_break

%type <node> statement
%type <list> statement_list

%%

input
    : statement_list
        {
            ASTTuneBody *n = node(TuneBody, @$);
            n->node.children = $1;
            *node = n;
        }
    ;

statement_list
    : statement
        {
            $$ = list();
            if ($1) {
                listAppend($$, $1);
            }
        }
    | statement_list statement
        {
            $$ = $1;
            if ($2) {
                listAppend($$, $2);
            }
        }
    ;

statement
    : inline_field
    | line_break
    | error
        {
            yyerrok;
            yyclearin;
            $$ = NULL;
        }
    | /* empty */
        {
            $$ = NULL;
        }
    ;

inline_field
    : INLINE_FIELD
        {
            TRACE("---- INLINE_FIELD [%s] %d - %d\n", $1, line, @$.first_column);
            $$ = ABCParserParseInformation(ABC_tune_body_get_extra(scanner), filepath, line, @$.first_column, $1);
            free($1);
        }
    ;

line_break
    : '\n'
        {
            $$ = node(LineBreak, @$);
        }
    ;

%%

int ABC_tune_body_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, int line, void **node, const char *message)
{
    FileLocation location = {(char *)filepath, line, yylloc->first_column};
    ABCParserSyntaxError(ABC_tune_body_get_extra(scanner), &location, ABC_tune_body_get_text(scanner));
    return 0;
}
