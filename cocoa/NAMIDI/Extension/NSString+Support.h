//
//  NSString+Support.h
//  NAMIDI
//
//  Created by abechan on 2016/04/30.
//  Copyright © 2016年 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSString (Support)
- (BOOL)isVersionSmallerThan:(NSString *)version;
- (BOOL)isVersionLargerThan:(NSString *)version;
@end
