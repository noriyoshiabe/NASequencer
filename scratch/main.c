#include "NAMidiParser.h"
#include <stdio.h>

static void onParseNote(void *context, uint32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity, uint32_t gatetime){ }
static void onParseTime(void *context, uint32_t tick, uint8_t numerator, uint8_t denominator){ }
static void onParseTempo(void *context, uint32_t tick, float tempo){ }
static void onParseMarker(void *context, uint32_t tick, const char *text){ }

static void onFinish(void *context, uint32_t length)
{
    printf("onFinish() length=%d\n", length);
}

static void onError(void *context, const char *filepath, int line, int column, ParseError error, ...)
{
    printf("onError() filepath=%s line=%d column=%d error=%d\n", filepath, line, column, error);
}

int main(int argc, char **argv)
{
    NAMidiParserCallbacks callbacks = {
        onParseNote,
        onParseTime,
        onParseTempo,
        onParseMarker,

        onFinish,
        onError,
    };

    NAMidiParser *parser = NAMidiParserCreate(&callbacks, NULL);
    NAMidiParserExecuteParse(parser, argv[1]);
    NAMidiParserDestroy(parser);
    return 0;
}
