#import "Sequence.h"

@interface Sequence() {
    TimeTable *_timeTable;
}

@property (nonatomic, readwrite) NSArray *events;

@end

@implementation Sequence

- (int32_t)resolution
{
    return TimeTableResolution(_timeTable);
}

- (int32_t)length
{
    return TimeTableLength(_timeTable);
}

- (TimeSign)timeSign:(int32_t)tick
{
    return TimeTableTimeSignOnTick(_timeTable, tick);
}

- (Location)location:(int32_t)tick
{
    // TODO
    Location l = {0,0,0};
    return l;
}

- (TimeTable *)timeTable
{
    return _timeTable;
}

- (NSArray *)eventsFrom:(int32_t)from to:(int32_t)to
{
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"tick BETWEEN {%d, %d}", from, to];
    return [self.events filteredArrayUsingPredicate:predicate];
}

- (id)initWithEvents:(NSArray *)events timeTable:(TimeTable *)timeTable
{
    if (self = [super init]) {
        self.events = events;
        _timeTable = TimeTableRetain(timeTable);
    }
    return self;
}

- (void)dealloc
{
    TimeTableRelease(_timeTable);
}

- (NSString *)description
{
    char *buffer = alloca(2048);
    TimeTableDumpToBuffer(_timeTable, buffer, 2048);
    return [NSString stringWithFormat:@"<%@\ntimeTable=%s\nevents=%@>", [self.class description], buffer, self.events];
}

@end
