//
//  Color.m
//  NAMIDI
//
//  Created by abechan on 2/17/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "Color.h"

#define RGBA(r,g,b,a) [NSColor colorWithCalibratedRed:r/255.0 green:r/255.0 blue:r/255.0 alpha:a/255.0]
#define RGB(r,g,b)    RGBA(r,g,b,255.0)
#define GSCALE(b)     RGBA(b,b,b,255.0)
#define HSBA(h,s,b,a) [NSColor  colorWithCalibratedHue:h saturation:s brightness:b alpha:a]
#define HSB(h,s,b)    HSBA(h,s,b,1.0)

static NSColor *DarkGray;
static NSColor *Gray;
static NSColor *LightGray;
static NSColor *ChannelColor[16];

@implementation Color

+ (void)initialize
{
    DarkGray = GSCALE(74.0);
    Gray = GSCALE(151.0);
    LightGray = GSCALE(216.0);
    
    for (int i = 0; i < 16; ++i) {
        ChannelColor[i] = HSB(1.0 / 16.0 * i, 1.0, 1.0);
    }
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

+ (NSColor *)channelColor:(int)channel
{
    return ChannelColor[channel - 1];
}

@end
