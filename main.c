/*
 * main.c file
 */
 
#include "ParseContext.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main(int argc, char **argv)
{
    ParseContext *parseContext;
    ParseError error;

    if (!(parseContext = ParseContextParse(argv[1], &error))) {
        printf("ParseError=%d %s\n", error.kind, error.filepath);
        printf("\t%d %d %d %d %s\n",
                error.location.firstLine,
                error.location.firstColumn,
                error.location.lastLine,
                error.location.lastColumn,
                error.message);
    }
    else {
        CFStringRef cfString = NADescription(parseContext);
        CFShow(cfString);
        CFRelease(cfString);
        NARelease(parseContext);
    }

    return 0;
}
