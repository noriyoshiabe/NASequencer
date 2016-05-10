//
//  MeasureScaleAssistant.m
//  NAMIDI
//
//  Created by abechan on 2/19/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MeasureScaleAssistant.h"

#define MINIMUM_MEASURE 128

static const CGFloat _zoomLevels[] = {
    // Geometric progression of 1.26
    0.20, 0.25, 0.31, 0.40, 0.50, 0.63, 0.79,
    1.00,
    1.26, 1.59, 2.00, 2.52, 3.18, 4.00, 5.04, 6.35, 8.00, 10.00
};
static const CGFloat _zoomLevelCount = sizeof(_zoomLevels) / sizeof(_zoomLevels[0]);

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

- (void)zoomIn
{
    for (int i = 0; i < _zoomLevelCount - 1; ++i) {
        if (_zoomLevels[i] <= _scale && _scale < _zoomLevels[i + 1]) {
            self.scale = _zoomLevels[i + 1];
            break;
        }
    }
}

- (void)zoomOut
{
    for (int i = 1; i < _zoomLevelCount; ++i) {
        if (_zoomLevels[i - 1] < _scale && _scale <= _zoomLevels[i]) {
            self.scale = _zoomLevels[i - 1];
            break;
        }
    }
}

- (void)zoomReset
{
    self.scale = 1.0;
}

@end
