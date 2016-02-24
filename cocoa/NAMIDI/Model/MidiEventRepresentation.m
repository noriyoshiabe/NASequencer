//
//  MidiEventRepresentation.m
//  NAMIDI
//
//  Created by abechan on 2/25/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MidiEventRepresentation.h"

@implementation MidiEventRepresentation

- (instancetype)initWithMidiEvent:(MidiEvent *)event
{
    self = [super init];
    if (self) {
        _raw = event;
    }
    return  self;
}

- (MidiEventType)type
{
    return _raw->type;
}

- (int)tick
{
    return _raw->tick;
}

- (int)channel
{
    return MidiEventGetChannel(_raw);
}

@end
