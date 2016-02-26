//
//  NSObject+Support.m
//  NAMIDI
//
//  Created by abechan on 2/27/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "NSObject+Support.h"

@implementation NSObject (Support)

- (void)notifyValueChangeForKey:(NSString *)key
{
    [self willChangeValueForKey:key];
    [self didChangeValueForKey:key];
}

@end
