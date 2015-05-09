#import "SequenceBuilder.h"
#import "MidiEvent.h"

@interface SequenceBuilder() {
    TimeTable *_timeTable;
}

@property (nonatomic, strong) NSMutableArray *events;

@end

@implementation SequenceBuilder

- (id)init
{
    if (self = [super init]) {
        self.events = [NSMutableArray array];
    }
    return self;
}

- (void)addEvent:(NAMidiParserEventType)type argList:(va_list)argList
{
    int32_t tick = va_arg(argList, int);

    switch (type) {
    case NAMidiParserEventTypeNote:
        {
            uint8_t channel = va_arg(argList, int);
            uint8_t noteNo = va_arg(argList, int);
            uint8_t velocity = va_arg(argList, int);
            uint32_t gatetime = va_arg(argList, int);
            [self.events addObject:[[NoteEvent alloc] initWithTick:tick channel:channel noteNo:noteNo velocity:velocity gatetime:gatetime]];
        }
        break;

    case NAMidiParserEventTypeTime:
        {
            uint8_t numerator = va_arg(argList, int);
            uint8_t denominator = va_arg(argList, int);
            [self.events addObject:[[TimeEvent alloc] initWithTick:tick numerator:numerator denominator:denominator]];
        }
        break;

    case NAMidiParserEventTypeTempo:
        {
            float tempo = va_arg(argList, double);
            [self.events addObject:[[TempoEvent alloc] initWithTick:tick tempo:tempo]];
        }
        break;

    case NAMidiParserEventTypeSound:
        {
            uint8_t channel = va_arg(argList, int);
            uint8_t msb = va_arg(argList, int);
            uint8_t lsb = va_arg(argList, int);
            uint8_t programNo = va_arg(argList, int);
            [self.events addObject:[[SoundEvent alloc] initWithTick:tick channel:channel msb:msb lsb:lsb programNo:programNo]];
        }
        break;

    case NAMidiParserEventTypeMarker:
        {
            const char *text = va_arg(argList, const char *);
            [self.events addObject:[[MarkerEvent alloc] initWithTick:tick text:text]];
        }
        break;
    }
}

- (void)setTimeTable:(TimeTable *)timeTable
{
    _timeTable = timeTable;
}

- (Sequence *)build
{
    Sequence *ret = [[Sequence alloc] initWithEvents:[self.events sortedArrayUsingSelector:@selector(compare:)] timeTable:_timeTable];
    self.events = nil;
    return ret;
}

@end
