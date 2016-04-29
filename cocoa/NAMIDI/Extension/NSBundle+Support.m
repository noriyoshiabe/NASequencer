//
//  NSBundle+Support.m
//  NAMIDI
//
//  Created by abechan on 2016/03/14.
//  Copyright © 2016年 Noriyoshi Abe. All rights reserved.
//

#import "NSBundle+Support.h"

@implementation NSBundle (Support)

+ (NSString *)shortVersionString
{
    return [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
}

+ (NSString *)bundleVersion
{
    return [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"];
}

+ (NSString *)versionString
{
    return [NSString stringWithFormat:@"Version %@ (%@)", self.shortVersionString, self.bundleVersion];
}

@end
