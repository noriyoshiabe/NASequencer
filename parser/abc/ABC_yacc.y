%{
 
#include "ABC_yacc.h"
#include "ABC_lex.h"

#include "ABCParser.h"
#include "ABCAST.h"

extern void ABC_lex_set_error_until_eol(yyscan_t scanner);
extern int ABC_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, void **node, const char *message);

extern Node *ABCParserParseIncludeFile(void *self, FileLocation *location, const char *includeFile);
extern void ABCParserSyntaxError(void *self, FileLocation *location, const char *token);

#define node(type, yylloc) ABCAST##type##Create(&((FileLocation){(char *)filepath, yylloc.first_line, yylloc.first_column}))
#define list() NAArrayCreate(4, NULL)
#define listAppend(list, node) NAArrayAppend(list, node)

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
    int i;
    float f;
    char *s;
    void *node;
    void *list;
}

%token TITLE
%token INSTRUCTION INCLUDE

%token<s> STRING
%token<s> VERSION NOTE FILEPATH

%type <node> version title note
%type <node> include line_break

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
    : version
    | title
    | note
    | line_break
    | include
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

version
    : VERSION
        {
            ASTVersion *n = node(Version, @$);
            n->versionString = $1;
            $$ = n;
        }
    ;

title
    : TITLE STRING
        {
            ASTTitle *n = node(Title, @$);
            n->title = $2;
            $$ = n;
        }
    ;

note
    : NOTE
        {
            ASTNote *n = node(Note, @$);
            n->noteString = $1;
            $$ = n;
        }
    ;

include
    : INSTRUCTION INCLUDE FILEPATH
        {
            ASTInclude *n = node(Include, @$);
            n->filepath = $3;
            FileLocation location = {(char *)filepath, @$.first_line, @$.first_column};
            n->root = ABCParserParseIncludeFile(ABC_get_extra(scanner), &location, $3);
            $$ = n;
        }
    ;

line_break
    : '\n'
        {
            $$ = node(LineBreak, @$);
        }
    ;

%%

int ABC_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, void **node, const char *message)
{
    FileLocation location = {(char *)filepath, yylloc->first_line, yylloc->first_column};
    ABCParserSyntaxError(ABC_get_extra(scanner), &location, ABC_get_text(scanner));
    return 0;
}
