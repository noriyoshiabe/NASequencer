//
//  NSString+Support.m
//  NAMIDI
//
//  Created by abechan on 2016/04/30.
//  Copyright © 2016年 Noriyoshi Abe. All rights reserved.
//

#import "NSString+Support.h"

@implementation NSString (Support)

- (BOOL)isVersionSmallerThan:(NSString *)version
{
    return NSOrderedAscending == [self compare:version options:NSNumericSearch];
}

- (BOOL)isVersionLargerThan:(NSString *)version
{
    return NSOrderedDescending == [self compare:version options:NSNumericSearch];
}

@end
