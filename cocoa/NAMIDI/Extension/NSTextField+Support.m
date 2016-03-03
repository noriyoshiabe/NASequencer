//
//  NSTextField+Support.m
//  NAMIDI
//
//  Created by abechan on 3/4/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "NSTextField+Support.h"

@implementation NSTextField (Support)

- (void)underLine
{
    NSMutableAttributedString *string = self.attributedStringValue.mutableCopy;
    [string addAttribute:NSUnderlineStyleAttributeName value:[NSNumber numberWithInt:NSUnderlineStyleSingle] range:NSMakeRange(0, string.length)];
    self.attributedStringValue = string;
}

@end
