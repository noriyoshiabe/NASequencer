//
//  NSView+Support.m
//  NAMIDI
//
//  Created by abechan on 2/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "NSView+Support.h"

@implementation NSView (Support)

- (void)showHierarchy
{
    NSMutableString *string = [NSMutableString string];
    [self showHierarchyWithView:self string:string indent:0];
    NSLog(@"\n%@\n", string);
}

- (void)showHierarchyWithView:(NSView *)view string:(NSMutableString *)string indent:(int)indent
{
    [string appendFormat:@"%*s%@\n", indent, "", NSStringFromClass(view.class)];
    
    for (NSView *child in view.subviews) {
        [self showHierarchyWithView:child string:string indent:indent + 4];
    }
}

- (void)addSubviewWithFitConstraints:(NSView *)aView
{
    aView.translatesAutoresizingMaskIntoConstraints = NO;
    
    [self addSubview:aView];
    [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:|[view]|" options:0 metrics:nil views:@{@"view": aView}]];
    [self addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[view]|" options:0 metrics:nil views:@{@"view": aView}]];
}

- (void)drawRoundedRect:(CGRect)rect constext:(CGContextRef)ctx rarius:(CGFloat)rarius fill:(BOOL)fill stroke:(BOOL)stroke
{
    CGFloat lx = CGRectGetMinX(rect);
    CGFloat cx = CGRectGetMidX(rect);
    CGFloat rx = CGRectGetMaxX(rect);
    CGFloat by = CGRectGetMinY(rect);
    CGFloat cy = CGRectGetMidY(rect);
    CGFloat ty = CGRectGetMaxY(rect);
    CGFloat r = MIN(rect.size.width / 2, rarius);
    
    if (fill) {
        CGContextMoveToPoint(ctx, lx, cy);
        CGContextAddArcToPoint(ctx, lx, by, cx, by, r);
        CGContextAddArcToPoint(ctx, rx, by, rx, cy, r);
        CGContextAddArcToPoint(ctx, rx, ty, cx, ty, r);
        CGContextAddArcToPoint(ctx, lx, ty, lx, cy, r);
        CGContextClosePath(ctx);
        
        CGContextFillPath(ctx);
    }
    
    if (stroke) {
        CGContextMoveToPoint(ctx, lx, cy);
        CGContextAddArcToPoint(ctx, lx, by, cx, by, r);
        CGContextAddArcToPoint(ctx, rx, by, rx, cy, r);
        CGContextAddArcToPoint(ctx, rx, ty, cx, ty, r);
        CGContextAddArcToPoint(ctx, lx, ty, lx, cy, r);
        CGContextClosePath(ctx);
        
        CGContextStrokePath(ctx);
    }
}

- (void)setBackgroundColor:(NSColor *)color
{
    self.wantsLayer = YES;
    self.layer.backgroundColor = color.CGColor;
}

@end
