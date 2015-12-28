%{
 
#include "ABC_yacc.h"
#include "ABC_lex.h"

#include "ABCParser.h"
#include "ABCAST.h"

extern int ABC_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, void **node, const char *message);

#define node(type, yylloc) ABCAST##type##Create(&((FileLocation){(char *)filepath, yylloc.first_line, yylloc.first_column}))
#define list() NAArrayCreate(4, NULL)
#define listAppend(list, node) NAArrayAppend(list, node)

#if 1
#define TRACE(...) printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

%}

%name-prefix = "ABC_"
%output = "ABC_yacc.c"
%defines

%pure-parser
%lex-param   { yyscan_t scanner }
%parse-param { yyscan_t scanner }
%parse-param { const char *filepath }
%parse-param { void **node }
%locations

%union {
    char *s;
    void *node;
    void *list;
}

%token <s>FILE_IDENTIFICATION
%token <s>INFORMATION
%token <s>DIRECTIVE
%token <s>TUNE_BODY

%token EMPTY_LINE

%type <node> file_identification
%type <node> information
%type <node> directive
%type <node> tune_body

%type <node> statement
%type <list> statement_list

%%

input
    : statement_list
        {
            ASTRoot *n = node(Root, @$);
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
    : file_identification
    | information
    | directive
    | tune_body
    ;

file_identification
    : FILE_IDENTIFICATION
        {
            TRACE("-- FILE_IDENTIFICATION [%s] %d - %d\n", $1, @$.first_line, @$.first_column);

            ASTFileIdentification *n = node(FileIdentification, @$);

            char *saveptr, *token, *s = $1;
            for (int i = 0; (token = strtok_r(s, "%-.", &saveptr)); ++i) {
                switch (i) {
                case 0:
                    n->identifier = strdup(token);
                    break;
                case 1:
                    n->major = atoi(token);
                    break;
                case 2:
                    n->minor = atoi(token);
                    break;
                default:
                    break;
                }
                s = NULL;
            }

            free($1);
            $$ = n;
        }
    ;

information
    : INFORMATION
        {
            TRACE("-- INFORMATION [%s] %d - %d\n", $1, @$.first_line, @$.first_column);
            $$ = ABCParserParseInformation(ABC_get_extra(scanner), filepath, @$.first_line, 0, $1);
            free($1);
        }
    ;

directive
    : DIRECTIVE
        {
            TRACE("-- DIRECTIVE [%s] %d - %d\n", $1, @$.first_line, @$.first_column);
            $$ = NULL;
            free($1);
        }
    ;

tune_body
    : TUNE_BODY
        {
            TRACE("-- TUNE_BODY [%s] %d - %d\n", $1, @$.first_line, @$.first_column);
            $$ = ABCParserParseTuneBody(ABC_get_extra(scanner), filepath, @$.first_line, 0, $1);
            free($1);
        }
    ;

%%

int ABC_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, void **node, const char *message)
{
    FileLocation location = {(char *)filepath, yylloc->first_line, yylloc->first_column};
    ABCParserSyntaxError(ABC_get_extra(scanner), &location, ABC_get_text(scanner));
    return 0;
}
