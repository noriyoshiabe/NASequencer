#pragma once

#import <Foundation/Foundation.h>

#import "TimeTable.h"

@interface Sequence : NSObject

@property (nonatomic, readonly) int32_t resolution;
@property (nonatomic, readonly) int32_t length;
@property (nonatomic, readonly) NSArray *events;

- (TimeSign)timeSign:(int32_t)tick;
- (Location)location:(int32_t)tick;

- (void)addNote:(uint32_t)tick channel:(uint8_t)channel noteNo:(uint8_t)noteNo velocity:(uint8_t)velocity gatetime:(uint32_t)gatetime;
- (void)addTime:(uint32_t)tick numerator:(uint8_t)numerator denominator:(uint8_t)denominator;
- (void)addTempo:(uint32_t)tick tempo:(float)tempo;
- (void)addMarker:(uint32_t)tick text:(const char *)text;
- (void)setTimeTable:(TimeTable *)timeTable;

- (void)build;

@end
