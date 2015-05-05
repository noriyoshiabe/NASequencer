#pragma once

#import <Foundation/Foundation.h>

#import "TimeTable.h"

@interface Sequence : NSObject

@property (nonatomic, readonly) int32_t resolution;
@property (nonatomic, readonly) int32_t length;
@property (nonatomic, readonly) NSArray *events;
@property (nonatomic, readonly) TimeTable *timeTable;

- (TimeSign)timeSign:(int32_t)tick;
- (Location)location:(int32_t)tick;
- (NSArray *)events:(int32_t)tickFrom tickTo:(int32_t)tickTo;

- (id)initWithEvents:(NSArray *)events timeTable:(TimeTable *)timeTable;

@end
