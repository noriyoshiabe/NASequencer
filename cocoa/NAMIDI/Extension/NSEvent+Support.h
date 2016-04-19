//
//  NSEvent+Support.h
//  NAMIDI
//
//  Created by abechan on 4/19/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSEvent (Support)
+ (BOOL)isAlternateKeyPressed;
+ (BOOL)isShiftKeyPressed;
@end
