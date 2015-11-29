#include "Parser.h"

#include <stdarg.h>

void ParserError(void *parser, FileLocation *location, int code, ...)
{
    printf("%s:%d:%d code=%d", location->filepath, location->line, location->column, code);

    va_list argList;
    va_start(argList, code);
    char *pc;
    while ((pc = va_arg(argList, char *))) {
        printf(" %s", pc);
    }
    va_end(argList);

    printf("\n");
}
