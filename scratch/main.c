#include <stdio.h>

#include "NAMidiParser.h"

int main(int argc, char **argv)
{
    NAMidiParser *parser = NAMidiParserCreate();
    NAMidiParserExecuteParse(parser, argv[1]);
    NAMidiParserDestroy(parser);
    return 0;
}
