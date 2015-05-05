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

@end
