#include "NAMidiParser.h"
#include <stdio.h>

static void onParseNote(void *receiver, uint32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity, uint32_t gatetime)
{
    printf("tick=%d channel=%d noteNo=%d velocity=%d gatetime=%d\n", tick, channel, noteNo, velocity, gatetime);
}

static void onParseTime(void *receiver, uint32_t tick, uint8_t numerator, uint8_t denominator){ }
static void onParseTempo(void *receiver, uint32_t tick, float tempo){ }
static void onParseMarker(void *receiver, uint32_t tick, const char *text){ }

static void onFinish(void *receiver, uint32_t length)
{
    printf("onFinish() length=%d\n", length);
}

static void onError(void *receiver, const char *filepath, int line, int column, ParseError error, ...)
{
    printf("onError() filepath=%s line=%d column=%d error=%s\n", filepath, line, column, ParseError2String(error));
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
