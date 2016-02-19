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
@property (readwrite, nonatomic) MidiEvent *raw;
@end

@implementation MidiEventRepresentation

- (MidiEventType)type
{
    return _raw->type;
}

- (int)tick
{
    return _raw->tick;
}

@end

@interface ChannelRepresentation ()
@property (readwrite, nonatomic) int number;
@property (readwrite, nonatomic) NoteRange noteRange;
@end

@implementation ChannelRepresentation

- (instancetype)init
{
    self = [super init];
    if (self) {
        _events = [NSMutableArray array];
    }
    return self;
}

- (void)dealloc
{
    // TODO remove
    for (MidiEventRepresentation *event in _events) {
        free(event.raw);
    }
}

@end

@interface SequenceRepresentation () {
    NSMutableArray *_eventsOfConductorTrack;
    NSMutableArray *_channels;
}

@end

@implementation SequenceRepresentation

- (instancetype)init
{
    self = [super init];
    if (self) {
        _eventsOfConductorTrack = [NSMutableArray array];
        _channels = [NSMutableArray array];
        
        MidiEventRepresentation *event;
        int _id = 0;
        
        event = [[MidiEventRepresentation alloc] init];
        event.raw = malloc(sizeof(TempoEvent));
        event.raw->id = ++_id;
        event.raw->type = MidiEventTypeTempo;
        event.raw->tick = 0;
        [_eventsOfConductorTrack addObject:event];
        
        event = [[MidiEventRepresentation alloc] init];
        event.raw = malloc(sizeof(TimeEvent));
        event.raw->id = ++_id;
        event.raw->type = MidiEventTypeTime;
        event.raw->tick = 0;
        [_eventsOfConductorTrack addObject:event];
        
        event = [[MidiEventRepresentation alloc] init];
        event.raw = malloc(sizeof(TempoEvent));
        event.raw->id = ++_id;
        event.raw->type = MidiEventTypeTempo;
        event.raw->tick = 1920 * 4;
        [_eventsOfConductorTrack addObject:event];
        
        for (int i = 0; i < 16; ++i) {
            ChannelRepresentation *channel = [[ChannelRepresentation alloc] init];
            channel.number = i + 1;
            channel.noteRange = (NoteRange){40, 68};
            for (int i = 0; i < 64; ++i) {
                event = [[MidiEventRepresentation alloc] init];
                NoteEvent *note = malloc(sizeof(NoteEvent));
                note->id = ++_id;
                note->type = MidiEventTypeNote;
                note->tick = 480 * i;
                note->channel = i + 1;
                note->noteNo = (2 * i) % 30 + 40;
                note->gatetime = 480;
                note->velocity = (2 * i) % 20 + 100;
                event.raw = (MidiEvent *)note;
                [(NSMutableArray *)channel.events addObject:event];
            }
            [_channels addObject:channel];
        }
    }
    return self;
}

- (void)dealloc
{
    // TODO remove
    for (MidiEventRepresentation *event in _eventsOfConductorTrack) {
        free(event.raw);
    }
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

@end