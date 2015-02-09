/*
 * main.c file
 */
 
#include "ParseContext.h"
#include "ConsoleWriter.h"

int main(int argc, char **argv)
{
    ParseContext *parseContext = ParseContextParse(argv[1]);
    ConsoleWriter *writer = NATypeNew(ConsoleWriter);

    ConsoleWriterWrite(writer, parseContext);

    NARelease(parseContext);
    NARelease(writer);

    return 0;
}
