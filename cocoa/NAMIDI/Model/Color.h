//
//  Color.h
//  NAMIDI
//
//  Created by abechan on 2/17/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface Color : NSObject
+ (NSColor *)ultraDarkGray;
+ (NSColor *)darkGray;
+ (NSColor *)gray;
+ (NSColor *)semiLightGray;
+ (NSColor *)lightGray;
+ (NSColor *)ultraLightGray;
+ (NSColor *)channelColor:(int)channel;
+ (NSColor *)statusBackground;
+ (NSColor *)grid;
+ (NSColor *)gridWeak;
+ (NSColor *)pink;
@end
