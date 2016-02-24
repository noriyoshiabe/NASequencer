//
//  MidiEventRepresentation.h
//  NAMIDI
//
//  Created by abechan on 2/25/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MidiEvent.h"

@interface MidiEventRepresentation : NSObject
@property (readonly, nonatomic) MidiEventType type;
@property (readonly, nonatomic) int tick;
@property (readonly, nonatomic) int channel;
@property (readonly, nonatomic) MidiEvent *raw;
- (instancetype)initWithMidiEvent:(MidiEvent *)event;
@end
