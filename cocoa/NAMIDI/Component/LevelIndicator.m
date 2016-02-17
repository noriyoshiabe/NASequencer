//
//  LevelIndicator.m
//  NAMIDI
//
//  Created by abechan on 2/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "LevelIndicator.h"

@interface LevelIndicator () {
    NSColor *red;
    NSColor *green;
    NSColor *offRed;
    NSColor *offGreen;
}

@end

@implementation LevelIndicator

- (void)awakeFromNib
{
    red = [NSColor redColor];
    green = [NSColor greenColor];
    offRed = [NSColor colorWithRed:1 green:0 blue:0 alpha:0.4];
    offGreen = [NSColor colorWithRed:0 green:1 blue:0 alpha:0.4];
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    CGContextRef ctx = [NSGraphicsContext currentContext].graphicsPort;
    CGContextSetLineWidth(ctx, 2.0);
    
    int level = self.floatValue * 50.0;
    CGFloat xPerLevel = self.bounds.size.width / 50.0;
    CGFloat height = self.bounds.size.height;
    
    for (int i = 1; i <= 50; ++i) {
        CGFloat x = (i - 1) * xPerLevel + 1;
        
        CGColorRef color;
        if (47 > i) {
            color = level < i ? offGreen.CGColor : green.CGColor;
        }
        else {
            color = level < i ? offRed.CGColor : red.CGColor;
        }
        
        CGContextSetStrokeColorWithColor(ctx, color);
        CGContextMoveToPoint(ctx, x, 0);
        CGContextAddLineToPoint(ctx, x, height);
        CGContextStrokePath(ctx);
    }
}

@end
