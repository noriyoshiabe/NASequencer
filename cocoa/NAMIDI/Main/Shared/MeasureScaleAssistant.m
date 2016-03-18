//
//  MeasureScaleAssistant.m
//  NAMIDI
//
//  Created by abechan on 2/19/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MeasureScaleAssistant.h"

#define MINIMUM_MEASURE 128

@implementation MeasureScaleAssistant

- (instancetype)init
{
    self = [super init];
    if (self) {
        _scale = 1.0;
    }
    return self;
}

- (CGFloat)measureOffset
{
    return 10.5;
}

- (CGFloat)pixelPerTick
{
    return 40.0 / _namidi.sequence.resolution * _scale;
}

- (CGFloat)tickPerPixel
{
    return 1.0 / self.pixelPerTick;
}

- (BOOL)scrollWheel:(NSEvent *)theEvent
{
    if (NSCommandKeyMask & theEvent.modifierFlags) {
        CGFloat scale = _scale + theEvent.deltaY * -0.05 * (1.0 > _scale ? pow(_scale, 2.0) : 1.0);
        self.scale = MAX(0.2, MIN(10.0, scale));
        return YES;
    }
    else {
        return NO;
    }
}

- (CGFloat)viewWidth
{
    return self.pixelPerTick * self.length + self.measureOffset * 2;
}

- (CGFloat)length
{
    return MAX(_namidi.sequence.length, _namidi.sequence.resolution * 4 * MINIMUM_MEASURE);
}

@end
