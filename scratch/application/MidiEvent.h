#pragma once

#import <Foundation/Foundation.h>

@interface MidiEvent : NSObject

@property (nonatomic, readonly) int32_t tick;

- (id)initWithTick:(int32_t)tick;

@end


@interface NoteEvent : MidiEvent

@property (nonatomic, readonly) uint8_t channel;
@property (nonatomic, readonly) uint8_t noteNo;
@property (nonatomic, readonly) uint8_t velocity;
@property (nonatomic, readonly) uint32_t gatetime;

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

@interface MarkerEvent : MidiEvent

@property (nonatomic, readonly) NSString *text;

- (id)initWithTick:(int32_t)tick text:(const char *)text;

@end
