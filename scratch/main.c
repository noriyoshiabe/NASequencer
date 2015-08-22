#include <stdio.h>

#include "NAMidiParser.h"

int main(int argc, char **argv)
{
    NAMidiParser *parser = NAMidiParserCreate();
    
    if (!NAMidiParserExecuteParse(parser, argv[1])) {
        const NAMidiParserError *error = NAMidiParserGetError(parser);
        printf("error:\n");
        printf("  kind: %s\n", NAMidiParserErrorKind2String(error->kind));
        printf("  filepath: %s\n", error->filepath);
        printf("  line: %d\n", error->line);
        printf("  column: %d\n", error->column);
    }

    const char **filepaths = NAMidiParserGetFilepaths(parser);
    const char *filepath;
    printf("files:\n");
    while ((filepath = *(filepaths++))) {
        printf("  %s\n", filepath);
    }

    NAMidiParserDestroy(parser);
    return 0;
}
