#pragma once

#import <Foundation/Foundation.h>

typedef NS_ENUM (NSUInteger, MidiEventType) {
    MidiEventTypeNote,
    MidiEventTypeTime,
    MidiEventTypeTempo,
    MidiEventTypeSound,
    MidiEventTypeMarker,
};

@interface MidiEvent : NSObject

@property (nonatomic, readonly) MidiEventType type;
@property (nonatomic, readonly) int32_t tick;

- (id)initWithTick:(int32_t)tick;

@end


@interface NoteEvent : MidiEvent

@property (nonatomic, readonly) uint8_t channel;
@property (nonatomic, readonly) uint8_t noteNo;
@property (nonatomic, readonly) uint8_t velocity;
@property (nonatomic, readonly) uint32_t gatetime;
@property (nonatomic, readonly) uint32_t offTick;

- (id)initWithTick:(int32_t)tick channel:(uint8_t)channel noteNo:(uint8_t)noteNo velocity:(uint8_t)velocity gatetime:(uint32_t)gatetime;

@end


@interface TimeEvent : MidiEvent

@property (nonatomic, readonly) uint16_t numerator;
@property (nonatomic, readonly) uint16_t denominator;

- (id)initWithTick:(int32_t)tick numerator:(uint16_t)numerator denominator:(uint16_t)denominator;

@end


@interface TempoEvent : MidiEvent

@property (nonatomic, readonly) float tempo;

- (id)initWithTick:(int32_t)tick tempo:(float)tempo;

@end

@interface SoundEvent : MidiEvent

@property (nonatomic, readonly) uint8_t channel;
@property (nonatomic, readonly) uint8_t msb;
@property (nonatomic, readonly) uint8_t lsb;
@property (nonatomic, readonly) uint8_t programNo;

- (id)initWithTick:(int32_t)tick channel:(uint8_t)channel msb:(uint8_t)msb lsb:(uint8_t)lsb programNo:(uint8_t)programNo;

@end

@interface MarkerEvent : MidiEvent

@property (nonatomic, readonly) NSString *text;

- (id)initWithTick:(int32_t)tick text:(const char *)text;

@end
