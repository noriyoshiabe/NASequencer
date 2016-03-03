//
//  SequenceRepresentation.h
//  NAMIDI
//
//  Created by abechan on 2/25/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MidiEventRepresentation.h"
#import "Sequence.h"

typedef struct _NoteRange {
    int16_t low;
    int16_t high;
} NoteRange;

@interface ChannelRepresentation : NSObject
@property (readonly, nonatomic) int number;
@property (readonly, nonatomic) BOOL exist;
@property (readonly, nonatomic) NSArray *events;
@property (readonly, nonatomic) NoteRange noteRange;
@end

@interface SequenceRepresentation : NSObject
@property (readonly, nonatomic) NSString *title;
@property (readonly, nonatomic) int32_t length;
@property (readonly, nonatomic) int32_t resolution;
@property (readonly, nonatomic) NSArray<MidiEventRepresentation *> *events;
@property (readonly, nonatomic) NSArray<MidiEventRepresentation *> *eventsOfConductorTrack;
@property (readonly, nonatomic) NSArray<ChannelRepresentation *> *channels;
- (instancetype)initWithSequence:(Sequence *)sequence;
- (TimeSign)timeSignByTick:(int)tick;
- (Location)locationByTick:(int)tick;
- (int)tickByLocation:(Location)location;
@end
