//
//  Stub.m
//  NAMIDI
//
//  Created by abechan on 2/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "Stub.h"

@implementation MidiSourceRepresentation

- (NSString *)name
{
    return [NSString stringWithFormat:@"Synth %p", self];
}

@end

@implementation PresetRepresentation

- (NSString *)name
{
    return [NSString stringWithFormat:@"Preset %p Bank:%d Prg:%d", self, self.bankNo, self.programNo];
}

- (uint16_t)bankNo
{
    return 120;
}

- (uint8_t)programNo
{
    return 2;
}

@end

@implementation PlayerRepresentation

- (int64_t)usec
{
    return 1 * 1000 * 1000;
}

- (int)tick
{
    return 960;
}

- (Location)location
{
    return (Location){1, 3, 0};
}

- (float)tempo
{
    return 124.50;
}

- (TimeSign)timeSign
{
    return (TimeSign){4, 4};
}

@end

@interface MidiEventRepresentation ()
@property (readwrite) MidiEventType type;
@property (readwrite) int tick;
@end

@implementation MidiEventRepresentation
@end

@interface SequenceRepresentation () {
    NSMutableArray *_eventsOfConductorTrack;
}
@end

@implementation SequenceRepresentation

- (instancetype)init
{
    self = [super init];
    if (self) {
        _eventsOfConductorTrack = [NSMutableArray array];
        
        MidiEventRepresentation *event;
        
        event = [[MidiEventRepresentation alloc] init];
        event.type = MidiEventTypeTempo;
        event.tick = 0;
        [_eventsOfConductorTrack addObject:event];
        
        event = [[MidiEventRepresentation alloc] init];
        event.type = MidiEventTypeTime;
        event.tick = 0;
        [_eventsOfConductorTrack addObject:event];
        
        event = [[MidiEventRepresentation alloc] init];
        event.type = MidiEventTypeTempo;
        event.tick = 1920 * 4;
        [_eventsOfConductorTrack addObject:event];
    }
    return self;
}

- (int32_t)length
{
    return 1920 * 16;
}

- (TimeSign)timeSignByTick:(int)tick
{
    return (TimeSign){4, 4};
}

- (Location)locationByTick:(int)tick
{
    return (Location){
        tick / 1920 + 1,
        (tick % 1920) / 480 + 1,
        tick % 480,
    };
}

- (int)tickByLocation:(Location)location
{
    return (location.m - 1) * 1920 + (location.b - 1) * 480 + location.t;
}

- (NSArray *)eventsOfConductorTrack
{
    return _eventsOfConductorTrack;
}

@end
