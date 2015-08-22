#include <stdio.h>

#include "NAMidiParser.h"

void _NAMidiParserRenderHandler(void *receiver, va_list argList)
{
    NAMidiParserEventType type = va_arg(argList, int);
    switch (type) {
    case NAMidiParserEventTypeNote:
        printf("%d:Note channel=%d noteNo=%d gatetime=%d velocity=%d\n",
                va_arg(argList, int), va_arg(argList, int), va_arg(argList, int), va_arg(argList, int), va_arg(argList, int));
        break;
    case NAMidiParserEventTypeTempo:
        printf("%d:Tempo %f\n", va_arg(argList, int), va_arg(argList, double));
        break;
    case NAMidiParserEventTypeTime:
        printf("%d:Time %d/%d\n", va_arg(argList, int), va_arg(argList, int), va_arg(argList, int));
        break;
    case NAMidiParserEventTypeSound:
        printf("%d:Sound channel=%d msb=%d lsb=%d programNo=%d\n",
                va_arg(argList, int), va_arg(argList, int), va_arg(argList, int), va_arg(argList, int), va_arg(argList, int));
        break;
    case NAMidiParserEventTypeMarker:
        printf("%d:Marker %s\n", va_arg(argList, int), va_arg(argList, char *));
        break;
    case NAMidiParserEventTypeTitle:
        printf("Title %s\n", va_arg(argList, char *));
        break;
    case NAMidiParserEventTypeVolume:
        printf("%d:Volume channel=%d volume=%d\n",
                va_arg(argList, int), va_arg(argList, int), va_arg(argList, int));
        break;
    case NAMidiParserEventTypePan:
        printf("%d:Pan channel=%d volume=%d\n",
                va_arg(argList, int), va_arg(argList, int), va_arg(argList, int));
        break;
    case NAMidiParserEventTypeChorus:
        printf("%d:Chorus channel=%d volume=%d\n",
                va_arg(argList, int), va_arg(argList, int), va_arg(argList, int));
        break;
    case NAMidiParserEventTypeReverb:
        printf("%d:Reverb channel=%d volume=%d\n",
                va_arg(argList, int), va_arg(argList, int), va_arg(argList, int));
        break;
    case NAMidiParserEventTypeKeySign:
        printf("%d:KeySign %s\n", va_arg(argList, int), va_arg(argList, char *));
        break;
    }
}

int main(int argc, char **argv)
{
    NAMidiParser *parser = NAMidiParserCreate(_NAMidiParserRenderHandler);
    
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
