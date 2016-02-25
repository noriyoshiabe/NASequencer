//
//  NSThread+Support.m
//  NAMIDI
//
//  Created by abechan on 2/25/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "NSThread+Support.h"

@implementation NSThread (Support)

+ (void)performBlockOnMainThread:(void (^)())block
{
    if ([NSThread isMainThread]) {
        block();
    }
    else {
        dispatch_async(dispatch_get_main_queue(), ^{
            block();
        });
    }
}

@end
