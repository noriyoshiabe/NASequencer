#import "NAMidi.h"
#import "NAMidiParser.h"
#import "SequenceBuilder.h"
#import "FSWatcher.h"

@interface NAMidi() {
    NAMidiParser *parser;
    FSWatcher *watcher;
}

@property (nonatomic, readwrite) Sequence *sequence;
@property (nonatomic, readwrite) Player *player;
@property (nonatomic, strong) SequenceBuilder *sequenceBuilder;

@property (nonatomic, strong) NSHashTable *observers;

@end

@implementation NAMidi

static void _NAMidiParserOnParseResolution(void *receiver, uint16_t resolution)
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

static void _NAMidiParserOnParseSound(void *receiver, uint32_t tick, uint8_t channel, uint8_t msb, uint8_t lsb, uint8_t programNo)
{
    NAMidi *self = (__bridge NAMidi *)receiver;
    [self parser:self->parser onParseSound:tick channel:channel msb:msb lsb:lsb programNo:programNo];
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

static NAMidiParserCallbacks parserCallbacks = {
    _NAMidiParserOnParseResolution,
    _NAMidiParserOnParseNote,
    _NAMidiParserOnParseTime,
    _NAMidiParserOnParseTempo,
    _NAMidiParserOnParseSound,
    _NAMidiParserOnParseMarker,

    _NAMidiParserOnFinish,
    _NAMidiParserOnError,
};

static void _FSWatcherOnFileChanged(void *receiver, const char *changedFile)
{
    NAMidi *self = (__bridge NAMidi *)receiver;
    [self watcher:self->watcher onFileChanged:changedFile];
}

static void _FSWatcherOnError(void *receiver, int error, const char *message)
{
    NAMidi *self = (__bridge NAMidi *)receiver;
    [self watcher:self->watcher onError:error message:message];
}

static FSWatcherCallbacks watcherCallbacks = {
    _FSWatcherOnFileChanged,
    _FSWatcherOnError,
};

- (id)init
{
    if (self = [super init]) {
        parser = NAMidiParserCreate(&parserCallbacks, (__bridge void *)self);
        watcher = FSWatcherCreate(&watcherCallbacks, (__bridge void *)self);

        self.observers = [NSHashTable weakObjectsHashTable];
        self.player = [[Player alloc] init];
        self.player.delegate = self;
    }
    return self;
}

- (void)dealloc
{
    FSWatcherFinish(watcher);
    FSWatcherDestroy(watcher);
    NAMidiParserDestroy(parser);

    self.sequence = nil;
    self.player = nil;
    self.sequenceBuilder = nil;
}

- (void)addObserver:(id<NAMidiObserver>)observer
{
    [self.observers addObject:observer];
}

- (void)removeObserver:(id<NAMidiObserver>)observer
{
    [self.observers removeObject:observer];
}

- (void)setFilepath:(NSString *)filepath
{
    _filepath = filepath;
    FSWatcherRegisterFilepath(self->watcher, [_filepath UTF8String]);
    FSWatcherStart(self->watcher);
}

- (void)parse
{
    self.sequenceBuilder = [[SequenceBuilder alloc] init];
    NAMidiParserExecuteParse(parser, [self.filepath UTF8String]);
    self.sequenceBuilder = nil;
}

- (void)stop
{
    [self.player stop];
}

- (void)play
{
    [self.player play];
}

- (void)playPause
{
    [self.player playPause];
}

- (void)rewind
{
    [self.player rewind];
}

- (void)forward
{
    [self.player forward];
}

- (void)backward
{
    [self.player backward];
}

- (void)parser:(NAMidiParser *)parser onParseResolution:(uint16_t)resolution
{
}

- (void)parser:(NAMidiParser *)parser onParseNote:(uint32_t)tick channel:(uint8_t)channel noteNo:(uint8_t)noteNo velocity:(uint8_t)velocity gatetime:(uint32_t)gatetime
{
    [self.sequenceBuilder addNote:tick channel:channel noteNo:noteNo velocity:velocity gatetime:gatetime];
}

- (void)parser:(NAMidiParser *)parser onParseTime:(uint32_t)tick numerator:(uint8_t)numerator denominator:(uint8_t)denominator
{
    [self.sequenceBuilder addTime:tick numerator:numerator denominator:denominator];
}

- (void)parser:(NAMidiParser *)parser onParseTempo:(uint32_t)tick tempo:(float)tempo
{
    [self.sequenceBuilder addTempo:tick tempo:tempo];
}

- (void)parser:(NAMidiParser *)parser onParseSound:(uint32_t)tick channel:(uint8_t)channel msb:(uint8_t)msb lsb:(uint8_t)lsb programNo:(uint8_t)programNo
{
    [self.sequenceBuilder addSound:tick channel:channel msb:msb lsb:lsb programNo:programNo];
}

- (void)parser:(NAMidiParser *)parser onParseMarker:(uint32_t)tick text:(const char *)text
{
    [self.sequenceBuilder addMarker:tick text:text];
}

- (void)parser:(NAMidiParser *)parser onFinish:(TimeTable *)timeTable
{
    [self.sequenceBuilder setTimeTable:timeTable];
    self.sequence = [self.sequenceBuilder build];
    self.player.sequence = self.sequence;

    for (id<NAMidiObserver> observer in self.observers) {
        if ([observer respondsToSelector:@selector(namidi:onParseFinish:)]) {
            [observer namidi:self onParseFinish:self.sequence];
        }
    }
}

- (void)parser:(NAMidiParser *)parser onError:(const char *)filepath line:(int)line column:(int)column error:(ParseError)error info:(const void *)info
{
    for (id<NAMidiObserver> observer in self.observers) {
        if ([observer respondsToSelector:@selector(namidi:onParseError:line:column:error:info:)]) {
            [observer namidi:self onParseError:[NSString stringWithUTF8String:filepath] line:line column:column error:error info:info];
        }
    }
}

- (void)watcher:(FSWatcher *)watcher onFileChanged:(const char *)changedFile
{
    // TODO post to main thread
    [self parse];
}

- (void)watcher:(FSWatcher *)watcher onError:(int)error message:(const char *)message
{
    printf("watcher:OnError message=%s\n", message);
}

- (void)player:(Player *)player notifyEvent:(PlayerEvent)playerEvent
{
    for (id<NAMidiObserver> observer in self.observers) {
        if ([observer respondsToSelector:@selector(namidi:player:notifyEvent:)]) {
            [observer namidi:self player:player notifyEvent:playerEvent];
        }
    }
}

- (void)player:(Player *)player didSendNoteOn:(NoteEvent *)noteEvent
{
    for (id<NAMidiObserver> observer in self.observers) {
        if ([observer respondsToSelector:@selector(namidi:player:didSendNoteOn:)]) {
            [observer namidi:self player:player didSendNoteOn:noteEvent];
        }
    }
}

- (void)player:(Player *)player didSendNoteOff:(NoteEvent *)noteEvent
{
    for (id<NAMidiObserver> observer in self.observers) {
        if ([observer respondsToSelector:@selector(namidi:player:didSendNoteOff:)]) {
            [observer namidi:self player:player didSendNoteOff:noteEvent];
        }
    }
}

@end
