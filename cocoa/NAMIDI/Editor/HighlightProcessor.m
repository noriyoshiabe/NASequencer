//
//  HighlightProcessor.m
//  NAMIDI
//
//  Created by abechan on 3/5/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "HighlightProcessor.h"

static NSColor *CommentColor;

@implementation HighlightColor

+ (void)initialize
{
    CommentColor = [NSColor colorWithHexRGBA:0x057523FF];
}

+ (NSColor *)comment
{
    return CommentColor;
}

@end
