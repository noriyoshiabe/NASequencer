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

static void _NAMidiParserOnParseEvent(void *receiver, NAMidiParserEventType type, ...)
{
    NAMidi *self = (__bridge NAMidi *)receiver;

    va_list argList;
    va_start(argList, type);

    [self parser:self->parser onParseEvent:type argList:argList];

    va_end(argList);
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
    _NAMidiParserOnParseEvent,
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

- (void)parser:(NAMidiParser *)parser onParseEvent:(NAMidiParserEventType)type argList:(va_list)argList
{
    int32_t tick = va_arg(argList, int);

    switch (type) {
    case NAMidiParserEventTypeNote:
        {
            uint8_t channel = va_arg(argList, int);
            uint8_t noteNo = va_arg(argList, int);
            uint8_t velocity = va_arg(argList, int);
            uint32_t gatetime = va_arg(argList, int);
            [self.sequenceBuilder addNote:tick channel:channel noteNo:noteNo velocity:velocity gatetime:gatetime];
        }
        break;
    case NAMidiParserEventTypeTime:
        {
            uint8_t numerator = va_arg(argList, int);
            uint8_t denominator = va_arg(argList, int);
            [self.sequenceBuilder addTime:tick numerator:numerator denominator:denominator];
        }
        break;
    case NAMidiParserEventTypeTempo:
        {
            float tempo = va_arg(argList, double);
            [self.sequenceBuilder addTempo:tick tempo:tempo];
        }
        break;
    case NAMidiParserEventTypeSound:
        {
            uint8_t channel = va_arg(argList, int);
            uint8_t msb = va_arg(argList, int);
            uint8_t lsb = va_arg(argList, int);
            uint8_t programNo = va_arg(argList, int);
            [self.sequenceBuilder addSound:tick channel:channel msb:msb lsb:lsb programNo:programNo];
        }
        break;
    case NAMidiParserEventTypeMarker:
        {
            const char *text = va_arg(argList, const char *);
            [self.sequenceBuilder addMarker:tick text:text];
        }
        break;
    }
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
