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

@end
