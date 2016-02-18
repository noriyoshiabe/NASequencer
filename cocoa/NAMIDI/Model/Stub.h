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

typedef struct _MidiEvent {
    MidiEventType type;
    int id;
    int tick;
} MidiEvent;

typedef struct _NoteEvent {
    MidiEventType type;
    int id;
    int tick;
    int channel;
    int noteNo;
    int gatetime;
    int velocity;
} NoteEvent;

typedef struct _TempoEvent {
    MidiEventType type;
    int id;
    int tick;
    float tempo;
} TempoEvent;

typedef struct _TimeEvent {
    MidiEventType type;
    int id;
    int tick;
    int numerator;
    int denominator;
} TimeEvent;

@interface MidiEventRepresentation : NSObject
@property (readonly, nonatomic) MidiEventType type;
@property (readonly, nonatomic) int tick;
@property (readonly, nonatomic) MidiEvent *raw;
@end

typedef struct _NoteRange {
    int16_t low;
    int16_t high;
} NoteRange;

@interface ChannelRepresentation : NSObject
@property (readonly, nonatomic) int number;
@property (readonly, nonatomic) NSArray *events;
@property (readonly, nonatomic) NoteRange noteRange;
@end

@interface SequenceRepresentation : NSObject
@property (readonly, nonatomic) int32_t length;
@property (readonly, nonatomic) NSArray<MidiEventRepresentation *> *eventsOfConductorTrack;
@property (readonly, nonatomic) NSArray<ChannelRepresentation *> *channels;
- (TimeSign)timeSignByTick:(int)tick;
- (Location)locationByTick:(int)tick;
- (int)tickByLocation:(Location)location;
@end
