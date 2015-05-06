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

- (void)addNote:(uint32_t)tick channel:(uint8_t)channel noteNo:(uint8_t)noteNo velocity:(uint8_t)velocity gatetime:(uint32_t)gatetime
{
    [self.events addObject:[[NoteEvent alloc] initWithTick:tick channel:channel noteNo:noteNo velocity:velocity gatetime:gatetime]];
}

- (void)addTime:(uint32_t)tick numerator:(uint8_t)numerator denominator:(uint8_t)denominator
{
    [self.events addObject:[[TimeEvent alloc] initWithTick:tick numerator:numerator denominator:denominator]];
}

- (void)addTempo:(uint32_t)tick tempo:(float)tempo
{
    [self.events addObject:[[TempoEvent alloc] initWithTick:tick tempo:tempo]];
}

- (void)addMarker:(uint32_t)tick text:(const char *)text
{
    [self.events addObject:[[MarkerEvent alloc] initWithTick:tick text:text]];
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
