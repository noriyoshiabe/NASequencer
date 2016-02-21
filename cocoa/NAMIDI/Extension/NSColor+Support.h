//
//  NSColor+Support.h
//  NAMIDI
//
//  Created by abechan on 2/21/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSColor (Support)
+ (NSColor *)colorWith8bitRed:(uint8_t)red green:(uint8_t)green blue:(uint8_t)blue alpha:(uint8_t)alpha;
+ (CGGradientRef)createVerticalGradientWithTopColor:(NSColor *)topColor bottomColor:(NSColor *)bottomColor;
- (NSColor *)inverseColor;
@end
