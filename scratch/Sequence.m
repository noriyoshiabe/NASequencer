#import "Sequence.h"

@interface Sequence() {
    TimeTable *timeTable;
}

@property (nonatomic, readwrite) NSArray *events;
@property (nonatomic, strong) NSMutableArray *buffer;

@end

@implementation Sequence

- (TimeSign)timeSign:(int32_t)tick
{
    TimeSign s = {4,4};
    return s;
}

- (Location)location:(int32_t)tick
{
    Location l = {0,0,0};
    return l;
}

- (void)addNote:(uint32_t)tick channel:(uint8_t)channel noteNo:(uint8_t)noteNo velocity:(uint8_t)velocity gatetime:(uint32_t)gatetime
{
}

- (void)addTime:(uint32_t)tick numerator:(uint8_t)numerator denominator:(uint8_t)denominator
{
}

- (void)addTempo:(uint32_t)tick tempo:(float)tempo
{
}

- (void)addMarker:(uint32_t)tick text:(const char *)text
{
}

- (void)setTimeTable:(TimeTable *)timeTable
{
}

- (void)build
{
}

@end
