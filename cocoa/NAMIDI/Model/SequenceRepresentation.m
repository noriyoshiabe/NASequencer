//
//  SequenceRepresentation.m
//  NAMIDI
//
//  Created by abechan on 2/25/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "SequenceRepresentation.h"

@interface ChannelRepresentation () {
    NSMutableArray *_events;
}

@property (readwrite, nonatomic) int number;
@property (readwrite, nonatomic) NoteRange noteRange;
- (NSMutableArray *)_events;
- (void)addEvent:(MidiEventRepresentation *)event;
@end

@implementation ChannelRepresentation

- (instancetype)init
{
    self = [super init];
    if (self) {
        _events = [NSMutableArray array];
        _noteRange.low = 127;
        _noteRange.high = 0;
    }
    return self;
}

- (BOOL)exist
{
    return 0 < [_events count];
}

- (NSMutableArray *)_events
{
    return (NSMutableArray *)_events;
}

- (void)addEvent:(MidiEventRepresentation *)event
{
    [_events addObject:event];
    
    if (MidiEventTypeNote == event.raw->type) {
        NoteEvent *note = (NoteEvent *)event.raw;
        _noteRange.low = MIN(note->noteNo, _noteRange.low);
        _noteRange.high = MAX(note->noteNo, _noteRange.high);
    }
}

@end

@interface SequenceRepresentation () {
    Sequence *_sequence;
    NSMutableArray *_events;
    NSMutableArray *_eventsOfConductorTrack;
    NSMutableArray *_channels;
}

@end

@implementation SequenceRepresentation

- (instancetype)initWithSequence:(Sequence *)sequence
{
    self = [super init];
    if (self) {
        _sequence = sequence;
        _events = [NSMutableArray array];
        _eventsOfConductorTrack = [NSMutableArray array];
        _channels = [NSMutableArray array];
        
        for (int i = 1; i <= 16; ++i) {
            ChannelRepresentation *channel = [[ChannelRepresentation alloc] init];
            channel.number = i;
            [_channels addObject:channel];
        }
        
        NAIterator *iterator = NAArrayGetIterator(sequence->events);
        while (iterator->hasNext(iterator)) {
            MidiEvent *raw = iterator->next(iterator);
            MidiEventRepresentation *event = [[MidiEventRepresentation alloc] initWithMidiEvent:raw];
            
            int channel = event.channel;
            if (0 == channel) {
                [_eventsOfConductorTrack addObject:event];
            }
            else {
                [_channels[channel - 1] addEvent:event];
            }
            
            [_events addObject:event];
        }
        
        SequenceRetain(_sequence);
    }
    return self;
}

- (void)dealloc
{
    if (_sequence) {
        SequenceRelease(_sequence);
    }
}

- (NSString *)title
{
    return [NSString stringWithUTF8String:_sequence->title];
}

- (int32_t)length
{
    return TimeTableLength(_sequence->timeTable);
}

- (int32_t)resolution
{
    return TimeTableResolution(_sequence->timeTable);
}

- (TimeSign)timeSignByTick:(int)tick
{
    return TimeTableTimeSignOnTick(_sequence->timeTable, tick);
}

- (Location)locationByTick:(int)tick
{
    return TimeTableTick2Location(_sequence->timeTable, tick);
}

- (int)tickByLocation:(Location)location
{
    return TimeTableTickByLocation(_sequence->timeTable, location);
}

@end
