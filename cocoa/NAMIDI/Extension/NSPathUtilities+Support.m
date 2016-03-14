//
//  NSPathUtilities+Support.m
//  NAMIDI
//
//  Created by abechan on 3/2/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "NSPathUtilities+Support.h"

#include <pwd.h>

NSString *NSUserHomeDirectory(void)
{
    return [NSString stringWithCString:getpwuid(getuid())->pw_dir encoding:NSUTF8StringEncoding];
}

NSString *NSUserMusicDirectory(void)
{
    return [NSUserHomeDirectory() stringByAppendingPathComponent:@"Music"];
}

NSString *NSApplicationHomeInMusicDirectory(void)
{
    return [NSUserMusicDirectory() stringByAppendingPathComponent:[[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleName"]];
}