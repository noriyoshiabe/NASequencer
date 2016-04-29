//
//  NSNull+Support.m
//  NAMIDI
//
//  Created by abechan on 2016/04/30.
//  Copyright © 2016年 Noriyoshi Abe. All rights reserved.
//

#import "NSNull+Support.h"

@implementation NSNull (Support)

+(BOOL)isNull:(id)obj
{
    return !obj || [[NSNull null] isEqual:obj];
}

@end
