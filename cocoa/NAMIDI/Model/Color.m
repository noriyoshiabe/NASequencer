//
//  Color.m
//  NAMIDI
//
//  Created by abechan on 2/17/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "Color.h"

#define RGBA(r,g,b,a) [NSColor colorWithRed:r/255.0 green:g/255.0 blue:b/255.0 alpha:a/255.0]
#define RGB(r,g,b)    RGBA(r,g,b,255.0)
#define GSCALE(b)     RGBA(b,b,b,255.0)
#define HSBA(h,s,b,a) [NSColor colorWithHue:h saturation:s brightness:b alpha:a]
#define HSB(h,s,b)    HSBA(h,s,b,1.0)

static NSColor *DarkGray;
static NSColor *Gray;
static NSColor *SemiLightGray;
static NSColor *LightGray;
static NSColor *UltraLightGray;
static NSColor *ChannelColor[16];
static NSColor *StatusBackground;
static NSColor *Grid;
static NSColor *GridWeak;
static NSColor *Pink;

@implementation Color

+ (void)initialize
{
    DarkGray = GSCALE(74.0);
    Gray = GSCALE(151.0);
    SemiLightGray = GSCALE(200.0);
    LightGray = GSCALE(216.0);
    UltraLightGray = GSCALE(242.0);
    
    for (int i = 0; i < 16; ++i) {
        int index = (32 - i + 9) % 16;
        ChannelColor[i] = HSB(1.0 / 16.0 * index, 1.0, 1.0);
    }
    
    StatusBackground = [NSColor colorWithWhite:0 alpha:0.5];
    Grid = RGBA(200.0, 200.0, 250.0, 255.0);
    GridWeak = RGBA(240.0, 240.0, 250.0, 255.0);
    
    Pink = RGBA(255.0, 66.0, 97.0, 255.0);
}

+ (NSColor *)darkGray
{
    return DarkGray;
}

+ (NSColor *)gray
{
    return Gray;
}

+ (NSColor *)semiLightGray
{
    return SemiLightGray;
}

+ (NSColor *)lightGray
{
    return LightGray;
}

+ (NSColor *)ultraLightGray
{
    return UltraLightGray;
}

+ (NSColor *)channelColor:(int)channel
{
    return ChannelColor[channel - 1];
}

+ (NSColor *)statusBackground
{
    return StatusBackground;
}

+ (NSColor *)grid
{
    return Grid;
}

+ (NSColor *)gridWeak
{
    return GridWeak;
}

+ (NSColor *)pink
{
    return Pink;
}

@end
