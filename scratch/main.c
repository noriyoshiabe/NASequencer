#include <stdio.h>

#include "NAMidiParser.h"

static void onParseEvent(void *receiver, NAMidiParserEvent *event, va_list argList)
{
    switch (event->type) {
    case NAMidiParserEventTypeNote:
        printf("Note: %s step=%d gatetime=%d velocity=%d\n", va_arg(argList, char *), va_arg(argList, int), va_arg(argList, int), va_arg(argList, int));
        break;
    case NAMidiParserEventTypeTempo:
        printf("Tempo: %f\n", va_arg(argList, double));
        break;
    case NAMidiParserEventTypeResolution:
        printf("Resolution: %d\n", va_arg(argList, int));
        break;
    case NAMidiParserEventTypeTimeSign:
        printf("Time: %d/%d\n", va_arg(argList, int), va_arg(argList, int));
        break;
    case NAMidiParserEventTypeMeasure:
        printf("Measure: %d\n", va_arg(argList, int));
        break;
    case NAMidiParserEventTypePattern:
        printf("Pattern: %s\n", va_arg(argList, char *));
        break;
    case NAMidiParserEventTypePatternDefine:
        printf("PatternDefine: %s\n", va_arg(argList, char *));
        break;
    case NAMidiParserEventTypeEnd:
        printf("End:\n");
        break;
    case NAMidiParserEventTypeTrack:
        printf("Track: %d\n", va_arg(argList, int));
        break;
    case NAMidiParserEventTypeChannel:
        printf("Channel: %d\n", va_arg(argList, int));
        break;
    case NAMidiParserEventTypeSound:
        printf("Sound: msb=%d lsb=%d programNo=%d\n", va_arg(argList, int), va_arg(argList, int), va_arg(argList, int));
        break;
    case NAMidiParserEventTypeMarker:
        printf("Marker: %s\n", va_arg(argList, char *));
        break;
    case NAMidiParserEventTypeTitle:
        printf("Title: %s\n", va_arg(argList, char *));
        break;
    case NAMidiParserEventTypeVolume:
        printf("Volume: %d\n", va_arg(argList, int));
        break;
    case NAMidiParserEventTypePan:
        printf("Pan: %d\n", va_arg(argList, int));
        break;
    case NAMidiParserEventTypeChorus:
        printf("Chorus: %d\n", va_arg(argList, int));
        break;
    case NAMidiParserEventTypeReverb:
        printf("Reverb: %d\n", va_arg(argList, int));
        break;
    case NAMidiParserEventTypeTranspose:
        printf("Transpose: %d\n", va_arg(argList, int));
        break;
    case NAMidiParserEventTypeKeySign:
        printf("KeySign: %s\n", va_arg(argList, char *));
        break;
    case NAMidiParserEventTypeRest:
        printf("Rest: step=%d\n", va_arg(argList, int));
        break;
    case NAMidiParserEventTypeIncludeFile:
        printf("IncludeFile: %s\n", va_arg(argList, char *));
        break;
    }
}

static void onParseError(void *receiver, NAMidiParserError *error)
{
    printf("error:\n");
    printf("  kind: %s\n", NAMidiParserErrorKind2String(error->kind));
    printf("  filepath: %s\n", error->location.filepath);
    printf("  line: %d\n", error->location.line);
    printf("  column: %d\n", error->location.column);
}

static NAMidiParserCallbacks callbacks = {onParseEvent, onParseError};

int main(int argc, char **argv)
{
    NAMidiParser *parser = NAMidiParserCreate(&callbacks, NULL);
    
    bool ret = NAMidiParserExecuteParse(parser, argv[1]);
    printf("-- ret: %d\n", ret);

    //printf("files:\n");
    //while ((const char *filepath = *(filepaths++))) {
    //    printf("  %s\n", filepath);
    //}

    NAMidiParserDestroy(parser);
    return 0;
}
