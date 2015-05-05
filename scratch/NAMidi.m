#import "NAMidi.h"
#import "NAMidiParser.h"

@interface NAMidi() {
    NAMidiParser *parser;
}

@end

@implementation NAMidi

static void onParseNote(void *receiver, uint32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity, uint32_t gatetime)
{
    printf("[Note] tick=%d channel=%d noteNo=%d velocity=%d gatetime=%d\n", tick, channel, noteNo, velocity, gatetime);
}

static void onParseTime(void *receiver, uint32_t tick, uint8_t numerator, uint8_t denominator)
{
    printf("[Time] tick=%d numerator=%d denominator=%d\n", tick, numerator, denominator);
}

static void onParseTempo(void *receiver, uint32_t tick, float tempo)
{
    printf("[Tempo] tick=%d tempo=%.2f\n", tick, tempo);
}

static void onParseMarker(void *receiver, uint32_t tick, const char *text)
{
    printf("[Marker] tick=%d text=%s\n", tick, text);
}

static void onFinish(void *receiver, uint32_t length)
{
    printf("onFinish() length=%d\n", length);
}

static void onError(void *receiver, const char *filepath, int line, int column, ParseError error, ...)
{
    printf("onError() filepath=%s line=%d column=%d error=%s\n", filepath, line, column, ParseError2String(error));
}

static NAMidiParserCallbacks callbacks = {
    onParseNote,
    onParseTime,
    onParseTempo,
    onParseMarker,

    onFinish,
    onError,
};

- (id)init
{
    if (self = [super init]) {
        parser = NAMidiParserCreate(&callbacks, (__bridge void *)self);
    }
    return self;
}

- (void)dealloc
{
    NAMidiParserDestroy(parser);
}

- (void)execute:(const char *)filepath
{
    NAMidiParserExecuteParse(parser, filepath);
}

@end
