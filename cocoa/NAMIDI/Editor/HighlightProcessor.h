//
//  HighlightProcessor.h
//  NAMIDI
//
//  Created by abechan on 3/5/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface HighlightSpec : NSObject
@property (strong, nonatomic) NSRegularExpression *regex;
@property (strong, nonatomic) NSColor *color;
@property (strong, nonatomic) HighlightSpec *next;
@end

@interface HighlightProcessor : NSObject
+ (void)processTextStorage:(NSTextStorage *)textStorage spec:(HighlightSpec *)spec;
@end

@interface HighlightColor : NSObject
+ (NSColor *)comment;
+ (NSColor *)directive;
+ (NSColor *)string;
+ (NSColor *)keyword;
+ (NSColor *)extra1;
@end
