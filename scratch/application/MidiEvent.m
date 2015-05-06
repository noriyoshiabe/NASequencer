#import "MidiEvent.h"

@interface MidiEvent()
@property (nonatomic, readwrite) int32_t tick;
@end

@implementation MidiEvent

- (id)initWithTick:(int32_t)tick
{
    if (self = [super init]) {
        self.tick = tick;
    }
    return self;
}

- (NSComparisonResult)compare:(MidiEvent *)event
{
    return self.tick - event.tick;
}

@end

@interface NoteEvent()
@property (nonatomic, readwrite) uint8_t channel;
@property (nonatomic, readwrite) uint8_t noteNo;
@property (nonatomic, readwrite) uint8_t velocity;
@property (nonatomic, readwrite) uint32_t gatetime;
@end

@implementation NoteEvent

- (id)initWithTick:(int32_t)tick channel:(uint8_t)channel noteNo:(uint8_t)noteNo velocity:(uint8_t)velocity gatetime:(uint32_t)gatetime
{
    if (self = [super initWithTick:tick]) {
        self.channel = channel;
        self.noteNo = noteNo;
        self.velocity = velocity;
        self.gatetime = gatetime;
    }
    return self;
}

- (MidiEventType)type
{
    return MidiEventTypeNote;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"%@ tick=%d channel=%d noteNo=%d velocity=%d gatetime=%d", self.class.description, self.tick, self.channel, self.noteNo, self.velocity, self.gatetime];
}

- (int32_t)offTick
{
    return self.tick + self.gatetime;
}

@end


@interface TimeEvent()
@property (nonatomic, readwrite) uint16_t numerator;
@property (nonatomic, readwrite) uint16_t denominator;
@end

@implementation TimeEvent

- (id)initWithTick:(int32_t)tick numerator:(uint16_t)numerator denominator:(uint16_t)denominator
{
    if (self = [super initWithTick:tick]) {
        self.numerator = numerator;
        self.denominator = denominator;
    }
    return self;
}

- (MidiEventType)type
{
    return MidiEventTypeTime;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"%@ tick=%d numerator=%d denominator=%d", self.class.description, self.tick, self.numerator, self.denominator];
}

@end


@interface TempoEvent()
@property (nonatomic, readwrite) float tempo;
@end

@implementation TempoEvent

- (id)initWithTick:(int32_t)tick tempo:(float)tempo
{
    if (self = [super initWithTick:tick]) {
        self.tempo = tempo;
    }
    return self;
}

- (MidiEventType)type
{
    return MidiEventTypeTempo;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"%@ tick=%d tempo=%.2f", self.class.description, self.tick, self.tempo];
}

@end


@interface MarkerEvent()
@property (nonatomic, readwrite) NSString *text;
@end

@implementation MarkerEvent

- (id)initWithTick:(int32_t)tick text:(const char *)text
{
    if (self = [super initWithTick:tick]) {
        self.text = [NSString stringWithCString:text encoding:NSUTF8StringEncoding];
    }
    return self;
}

- (MidiEventType)type
{
    return MidiEventTypeMarker;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"%@ tick=%d text=%@", self.class.description, self.tick, self.text];
}

@end
