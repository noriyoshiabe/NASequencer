//
//  NSColor+DarkNoMe.m
//  NAMIDI
//
//  Created by abechan on 2020/02/23.
//  Copyright © 2020 Noriyoshi Abe. All rights reserved.
//

#import "NSColor+DarkMode.h"

@implementation NSColor (DarkMode)

+(BOOL)isDarkMode
{
    if (@available(macOS 10.14, *)) {
        return NSApplication.sharedApplication.effectiveAppearance.name == NSAppearanceNameDarkAqua;
    }
    else {
        return NO;
    }
}

@end
