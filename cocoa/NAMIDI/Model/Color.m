//
//  Color.m
//  NAMIDI
//
//  Created by abechan on 2/17/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "Color.h"

#define RGBA(r,g,b,a) [NSColor colorWithRed:r/255.0 green:r/255.0 blue:r/255.0 alpha:a/255.0]
#define RGB(r,g,b)    RGBA(r,g,b,255.0)
#define GSCALE(b)    RGBA(b,b,b,255.0)

static NSColor *DarkGray;
static NSColor *Gray;
static NSColor *LightGray;

@implementation Color

+ (void)initialize
{
    DarkGray = GSCALE(74.0);
    Gray = GSCALE(151.0);
    LightGray = GSCALE(216.0);
}

+ (NSColor *)darkGray
{
    return DarkGray;
}

+ (NSColor *)gray
{
    return Gray;
}

+ (NSColor *)lightGray
{
    return LightGray;
}

@end
