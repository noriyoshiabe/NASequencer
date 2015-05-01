#include "NAMidiParser.h"

int main(int argc, char **argv)
{
    NAMidiParser *parser = NAMidiParserCreate(NULL);
    NAMidiParserExecuteParse(parser, argv[1]);
    NAMidiParserDestroy(parser);
    return 0;
}
