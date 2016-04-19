//
//  NSEvent+Support.m
//  NAMIDI
//
//  Created by abechan on 4/19/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "NSEvent+Support.h"

@implementation NSEvent (Support)

+ (BOOL)isAlternateKeyPressed
{
    return 0 != ([NSEvent modifierFlags] & NSAlternateKeyMask);
}

+ (BOOL)isShiftKeyPressed
{
    return 0 != ([NSEvent modifierFlags] & NSShiftKeyMask);
}

@end
