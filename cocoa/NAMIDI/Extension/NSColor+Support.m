//
//  NSColor+Support.m
//  NAMIDI
//
//  Created by abechan on 2/21/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "NSColor+Support.h"

@implementation NSColor (Support)

+ (NSColor *)colorWith8bitRed:(uint8_t)red green:(uint8_t)green blue:(uint8_t)blue alpha:(uint8_t)alpha
{
    return [NSColor colorWithRed:red / 255.0 green:green / 255.0 blue:blue / 255.0 alpha:alpha / 255.0];
}

+ (CGGradientRef)createVerticalGradientWithTopColor:(NSColor *)topColor bottomColor:(NSColor *)bottomColor
{
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGFloat components[] = {
        topColor.redComponent, topColor.greenComponent, topColor.blueComponent, topColor.alphaComponent,
        bottomColor.redComponent, bottomColor.greenComponent, bottomColor.blueComponent, bottomColor.alphaComponent,
    };
    CGFloat locations[] = {0.0f, 1.0f};
    size_t count = sizeof(components) / (sizeof(CGFloat) * 4);
    
    CGGradientRef gradient = CGGradientCreateWithColorComponents(colorSpaceRef, components, locations, count);
    
    CGColorSpaceRelease(colorSpaceRef);
    
    return gradient;
}

- (NSColor *)inverseColor
{
    CGFloat r,g,b,a;
    [self getRed:&r green:&g blue:&b alpha:&a];
    return [NSColor colorWithRed:1.0 - r green:1.0 -g blue:1.0 -b alpha:a];
}

@end
