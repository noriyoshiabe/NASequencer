#import "NAMidi.h"
#import "NAMidiParser.h"
#import "SequenceBuilder.h"

@interface NAMidi() {
    NAMidiParser *parser;
}

@property (nonatomic, strong) Sequence *sequence;
@property (nonatomic, strong) SequenceBuilder *sequenceBuilder;

@end

@implementation NAMidi

static void _NAMidiParserOnParseResolution(void *receiver, uint32_t resolution)
{
    NAMidi *self = (__bridge NAMidi *)receiver;
    [self parser:self->parser onParseResolution:resolution];
}

static void _NAMidiParserOnParseNote(void *receiver, uint32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity, uint32_t gatetime)
{
    NAMidi *self = (__bridge NAMidi *)receiver;
    [self parser:self->parser onParseNote:tick channel:channel noteNo:noteNo velocity:velocity gatetime:gatetime];
}

static void _NAMidiParserOnParseTime(void *receiver, uint32_t tick, uint8_t numerator, uint8_t denominator)
{
    NAMidi *self = (__bridge NAMidi *)receiver;
    [self parser:self->parser onParseTime:tick numerator:numerator denominator:denominator];
}

static void _NAMidiParserOnParseTempo(void *receiver, uint32_t tick, float tempo)
{
    NAMidi *self = (__bridge NAMidi *)receiver;
    [self parser:self->parser onParseTempo:tick tempo:tempo];
}

static void _NAMidiParserOnParseMarker(void *receiver, uint32_t tick, const char *text)
{
    NAMidi *self = (__bridge NAMidi *)receiver;
    [self parser:self->parser onParseMarker:tick text:text];
}

static void _NAMidiParserOnFinish(void *receiver, TimeTable *timeTable)
{
    NAMidi *self = (__bridge NAMidi *)receiver;
    [self parser:self->parser onFinish:timeTable];
}

static void _NAMidiParserOnError(void *receiver, const char *filepath, int line, int column, ParseError error, const void *info)
{
    NAMidi *self = (__bridge NAMidi *)receiver;
    [self parser:self->parser onError:filepath line:line column:column error:error info:info];
}

static NAMidiParserCallbacks callbacks = {
    _NAMidiParserOnParseResolution,
    _NAMidiParserOnParseNote,
    _NAMidiParserOnParseTime,
    _NAMidiParserOnParseTempo,
    _NAMidiParserOnParseMarker,

    _NAMidiParserOnFinish,
    _NAMidiParserOnError,
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
    self.sequenceBuilder = [[SequenceBuilder alloc] init];
    NAMidiParserExecuteParse(parser, filepath);
    self.sequenceBuilder = nil;
}

- (void)parser:(NAMidiParser *)parser onParseResolution:(uint32_t)resolution
{
    printf("[Resolution] resolution=%d\n", resolution);
}

- (void)parser:(NAMidiParser *)parser onParseNote:(uint32_t)tick channel:(uint8_t)channel noteNo:(uint8_t)noteNo velocity:(uint8_t)velocity gatetime:(uint32_t)gatetime
{
    printf("[Note] tick=%d channel=%d noteNo=%d velocity=%d gatetime=%d\n", tick, channel, noteNo, velocity, gatetime);
    [self.sequenceBuilder addNote:tick channel:channel noteNo:noteNo velocity:velocity gatetime:gatetime];
}

- (void)parser:(NAMidiParser *)parser onParseTime:(uint32_t)tick numerator:(uint8_t)numerator denominator:(uint8_t)denominator
{
    printf("[Time] tick=%d numerator=%d denominator=%d\n", tick, numerator, denominator);
    [self.sequenceBuilder addTime:tick numerator:numerator denominator:denominator];
}

- (void)parser:(NAMidiParser *)parser onParseTempo:(uint32_t)tick tempo:(float)tempo
{
    printf("[Tempo] tick=%d tempo=%.2f\n", tick, tempo);
    [self.sequenceBuilder addTempo:tick tempo:tempo];
}

- (void)parser:(NAMidiParser *)parser onParseMarker:(uint32_t)tick text:(const char *)text
{
    printf("[Marker] tick=%d text=%s\n", tick, text);
    [self.sequenceBuilder addMarker:tick text:text];
}

- (void)parser:(NAMidiParser *)parser onFinish:(TimeTable *)timeTable
{
    printf("onFinish()\n");
    TimeTableDump(timeTable);
    [self.sequenceBuilder setTimeTable:timeTable];
    self.sequence = [self.sequenceBuilder build];
}

- (void)parser:(NAMidiParser *)parser onError:(const char *)filepath line:(int)line column:(int)column error:(ParseError)error info:(const void *)info
{
    printf("onError() filepath=%s line=%d column=%d error=%s\n", filepath, line, column, ParseError2String(error));
}

@end
