//
//  HighlightProcessor.h
//  NAMIDI
//
//  Created by abechan on 3/5/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol HighlightProcessor <NSObject>
- (void)processTextStorage:(NSTextStorage *)textStorage;
@end

@interface HighlightColor : NSObject
+ (NSColor *)comment;
@end

