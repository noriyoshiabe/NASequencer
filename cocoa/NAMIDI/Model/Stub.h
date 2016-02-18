//
//  Stub.h
//  NAMIDI
//
//  Created by abechan on 2/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface MidiSourceRepresentation : NSObject
@property (readonly, nonatomic) NSString *name;
@end

@interface PresetRepresentation : NSObject
@property (readonly, nonatomic) NSString *name;
@property (readonly, nonatomic) uint16_t bankNo;
@property (readonly, nonatomic) uint8_t programNo;
@end

typedef struct _Location {
    int32_t m;
    int16_t b;
    int16_t t;
} Location;

typedef struct _TimeSign {
    int16_t numerator;
    int16_t denominator;
} TimeSign;

@interface PlayerRepresentation : NSObject
@property (readonly, nonatomic) int64_t usec;
@property (readonly, nonatomic) int tick;
@property (readonly, nonatomic) Location location;
@property (readonly, nonatomic) float tempo;
@property (readonly, nonatomic) TimeSign timeSign;
@end

typedef enum {
    MidiEventTypeNote,
    MidiEventTypeTempo,
    MidiEventTypeTime,
    MidiEventTypeKey,
    MidiEventTypeTitle,
    MidiEventTypeCopyright,
    MidiEventTypeMarker,
    MidiEventTypeVoice,
    MidiEventTypeVolume,
    MidiEventTypePan,
    MidiEventTypeChorus,
    MidiEventTypeReverb,
    MidiEventTypeExpression,
    MidiEventTypeDetune,
    
    MidiEventTypeSynth,
} MidiEventType;

@interface MidiEventRepresentation : NSObject
@property (readonly, nonatomic) MidiEventType type;
@property (readonly, nonatomic) int tick;
@end

@interface SequenceRepresentation : NSObject
@property (readonly, nonatomic) int32_t length;
- (TimeSign)timeSignByTick:(int)tick;
- (Location)locationByTick:(int)tick;
- (int)tickByLocation:(Location)location;
- (NSArray *)eventsOfConductorTrack;
@end
