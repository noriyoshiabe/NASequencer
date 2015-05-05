#import "SequenceBuilder.h"

@interface SequenceBuilder() {
    TimeTable *timeTable;
}

@property (nonatomic, strong) NSMutableArray *events;

@end

@implementation SequenceBuilder

- (void)addNote:(uint32_t)tick channel:(uint8_t)channel noteNo:(uint8_t)noteNo velocity:(uint8_t)velocity gatetime:(uint32_t)gatetime
{
    // TODO
}

- (void)addTime:(uint32_t)tick numerator:(uint8_t)numerator denominator:(uint8_t)denominator
{
    // TODO
}

- (void)addTempo:(uint32_t)tick tempo:(float)tempo
{
    // TODO
}

- (void)addMarker:(uint32_t)tick text:(const char *)text
{
    // TODO
}

- (void)setTimeTable:(TimeTable *)timeTable
{
    // TODO
}

- (Sequence *)build
{
    // TODO
    return [[Sequence alloc] init];
}

@end
