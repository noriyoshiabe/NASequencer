/*
 * main.c file
 */
 
#include "Expression.h"
#include "Parser.h"
#include "Lexer.h"
 
#include <stdio.h>
 
int yyparse(Expression **expression, yyscan_t scanner);
 
int main(void)
{
    Expression *expr = NULL;

    yyscan_t scanner;

    if (yylex_init(&scanner)) {
        printf("## error - %d\n", __LINE__);
        return -1;
    }

    FILE *fp = fopen("../example.namidi", "r");

    yyset_in(fp, scanner);

    YY_BUFFER_STATE state = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);

    if (yyparse(&expr, scanner)) {
        printf("## error - %d\n", __LINE__);
        return -1;
    }

    yy_delete_buffer(state, scanner);

    fclose(fp);

    yylex_destroy(scanner);
 
    dumpExpression(expr);
    deleteExpression(expr);
 
    return 0;
}
