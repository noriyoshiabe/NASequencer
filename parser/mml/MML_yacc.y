%{
 
#include "MML_yacc.h"
#include "MML_lex.h"

#include "MMLParser.h"
#include "MMLAST.h"

#include "NACString.h"

extern int MML_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, void **node, const char *message);

extern Node *MMLParserParseIncludeFile(void *self, FileLocation *location, const char *includeFile, ASTInclude *includeNode);
extern void MMLParserSyntaxError(void *self, FileLocation *location, const char *token);
extern void MMLParserUnExpectedEOF(void *self, FileLocation *location);

#define node(type, yylloc) MMLAST##type##Create(&((FileLocation){(char *)filepath, yylloc.first_line, yylloc.first_column}))
#define list() NAArrayCreate(4, NULL)
#define listAppend(list, node) NAArrayAppend(list, node)

%}

%name-prefix = "MML_"
%output = "MML_yacc.c"
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

%token <i>INTEGER
%token <f>FLOAT
%token <s>STRING

%token END_OF_FILE 0

%type <node> statement
%type <list> statement_list

%destructor { free($$); } STRING

%%

input
    : /* empty */
        {
            ASTRoot *n = node(Root, @$);
            n->node.children = list();
            *node = n;
        }
    | statement_list
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
    : error
        {
            yyclearin;
            $$ = NULL;
        }
    ;

%%

int MML_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, void **node, const char *message)
{
    FileLocation location = {(char *)filepath, yylloc->first_line, yylloc->first_column};
    MMLParserSyntaxError(MML_get_extra(scanner), &location, MML_get_text(scanner));
    return 0;
}
