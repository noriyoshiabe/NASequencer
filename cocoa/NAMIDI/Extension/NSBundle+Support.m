//
//  NSBundle+Support.m
//  NAMIDI
//
//  Created by abechan on 2016/03/14.
//  Copyright © 2016年 Noriyoshi Abe. All rights reserved.
//

#import "NSBundle+Support.h"

@implementation NSBundle (Support)

+ (NSString *)versionString
{
    NSString *shortVersionString = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
    NSString *bundleVersion = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"];
    return [NSString stringWithFormat:@"Version %@ (%@)", shortVersionString, bundleVersion];
}

@end
