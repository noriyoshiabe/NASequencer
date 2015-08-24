#include <stdio.h>

#include "NAMidiParser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>

typedef struct _Context {
    NAMidiParser *parser;
} Context;

static char *getRealPath(const char *filepath);
static char *buildPathWithDirectory(const char *directory, const char *filepath);
static char *getCurrentDirectory();

static void onParseEvent(void *receiver, NAMidiParserEvent *event, va_list argList)
{
    Context *context = receiver;

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
        {
            char *filename = va_arg(argList, char *);
            printf("IncludeFile: %s\n", filename);
            char *directory;
            
            if (event->location.filepath) {
                directory = dirname((char *)event->location.filepath);
            }
            else {
                directory = getCurrentDirectory();
            }

            char *fullPath = buildPathWithDirectory(directory, filename);

            FILE *fp = fopen(fullPath, "r");
            if (!fp) {
                printf("file not found. %s\n", fullPath);
            }
            else {
                bool ret = NAMidiParserExecuteParse(context->parser, fp, fullPath);
                printf("-- ret: %d\n", ret);
                fclose(fp);
            }
        }
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
    Context context;
    NAMidiParser *parser = NAMidiParserCreate(&callbacks, &context);
    context.parser = parser;

    FILE *fp = NULL;
    char *filepath = NULL;

    if (1 < argc) {
        filepath = getRealPath(argv[1]);
        fp = fopen(filepath, "r");
        if (!fp) {
            printf("file not found. %s\n", argv[1]);
            return 1;
        }
    }
    else {
        fp = stdin;
    }
    
    bool ret = NAMidiParserExecuteParse(parser, fp, filepath);
    printf("-- ret: %d\n", ret);
    if (fp != stdin) {
        fclose(fp);
    }

    //printf("files:\n");
    //while ((const char *filepath = *(filepaths++))) {
    //    printf("  %s\n", filepath);
    //}

    NAMidiParserDestroy(parser);
    return 0;
}

static char *getRealPath(const char *filepath)
{
    char buf[PATH_MAX];
    char *_filepath = realpath(filepath, buf);
    if (!_filepath) {
        return NULL;
    }

    char *ret = malloc(strlen(_filepath) + 1);
    strcpy(ret, _filepath);
    return ret;
}

static char *buildPathWithDirectory(const char *directory, const char *filename)
{
    char buf[PATH_MAX];
    snprintf(buf, PATH_MAX, "%s/%s", directory, filename);
    char *ret = malloc(strlen(buf) + 1);
    strcpy(ret, buf);
    return ret;
}

static char *getCurrentDirectory()
{
    char buf[PATH_MAX];
    char *directory = getcwd(buf, PATH_MAX);
    char *ret = malloc(strlen(directory) + 1);
    strcpy(ret, directory);
    return ret;
}
