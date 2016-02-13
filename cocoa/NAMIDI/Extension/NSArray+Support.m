//
//  NSArray+Support.m
//  NAMIDI
//
//  Created by abechan on 2/13/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "NSArray+Support.h"

@implementation NSArray (Support)

- (NSArray *)mapObjectsUsingBlock:(id (^)(id obj))block
{
    NSMutableArray *result = [NSMutableArray arrayWithCapacity:[self count]];
    for (id obj in self) {
        [result addObject:block(obj)];
    }
    return result;
}

@end
