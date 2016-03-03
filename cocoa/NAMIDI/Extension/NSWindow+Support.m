//
//  NSWindow+Support.m
//  NAMIDI
//
//  Created by abechan on 3/4/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "NSWindow+Support.h"

@implementation NSWindow (Support)

- (void)setFlippedFrameTopLeftPoint:(NSPoint)aPoint
{
    aPoint.y = self.screen.frame.size.height - aPoint.y;
    [self setFrameTopLeftPoint:aPoint];
}

@end
