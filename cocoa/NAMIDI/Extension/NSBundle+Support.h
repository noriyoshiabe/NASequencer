//
//  NSBundle+Support.h
//  NAMIDI
//
//  Created by abechan on 2016/03/14.
//  Copyright © 2016年 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSBundle (Support)
+ (NSString *)shortVersionString;
+ (NSString *)bundleVersion;
+ (NSString *)versionString;
@end
