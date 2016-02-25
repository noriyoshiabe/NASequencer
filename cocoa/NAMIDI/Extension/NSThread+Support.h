//
//  NSThread+Support.h
//  NAMIDI
//
//  Created by abechan on 2/25/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSThread (Support)
+ (void)performBlockOnMainThread:(void (^)())block;
@end
