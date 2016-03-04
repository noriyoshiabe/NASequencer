//
//  HighlightProcessor.m
//  NAMIDI
//
//  Created by abechan on 3/5/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "HighlightProcessor.h"

@implementation HighlightSpec

- (NSString *)description
{
    NSMutableString *string = [NSMutableString string];
    [string appendFormat:@"<HighlightSpec %p\n", self];
    [string appendFormat:@"\tregex=%@", _regex.pattern];
    [string appendFormat:@"\tcolor=%@", _color];
    [string appendFormat:@"\tnext=%@ >", _next];
    return string;
}

@end

@implementation HighlightProcessor

+ (void)processTextStorage:(NSTextStorage *)textStorage spec:(HighlightSpec *)spec
{
    NSRange range = NSMakeRange(0, textStorage.length);
    [textStorage removeAttribute:NSForegroundColorAttributeName range:range];
    [self processTextStorage:textStorage spec:spec range:range];
}

+ (void)processTextStorage:(NSTextStorage *)textStorage spec:(HighlightSpec *)spec range:(NSRange)range
{
    __block NSUInteger prevLocation = range.location;
    
    [spec.regex enumerateMatchesInString:textStorage.string options:0 range:range usingBlock:^(NSTextCheckingResult * _Nullable result, NSMatchingFlags flags, BOOL * _Nonnull stop) {
        
        NSRange matchedRange = [result rangeAtIndex:0];
        [textStorage addAttribute:NSForegroundColorAttributeName value:spec.color range:matchedRange];
        
        if (spec.next && prevLocation < matchedRange.location) {
            [self processTextStorage:textStorage spec:spec.next range:NSMakeRange(prevLocation, matchedRange.location - prevLocation)];
        }
        
        prevLocation = matchedRange.location + matchedRange.length;
    }];
    
    if (spec.next) {
        if (prevLocation == range.location) {
            [self processTextStorage:textStorage spec:spec.next range:range];
        }
        else if (prevLocation < range.location + range.length) {
            [self processTextStorage:textStorage spec:spec.next range:NSMakeRange(prevLocation, range.location + range.length - prevLocation)];
        }
    }
}

@end

static NSColor *CommentColor;
static NSColor *DirectiveColor;
static NSColor *StringColor;
static NSColor *KeywordColor;
static NSColor *Extra1Color;

@implementation HighlightColor

+ (void)initialize
{
    CommentColor = [NSColor colorWithHexRGBA:0x057523FF];
    DirectiveColor = [NSColor colorWithHexRGBA:0x151B88FF];
    StringColor = [NSColor colorWithHexRGBA:0xDB2023FF];
    KeywordColor = [NSColor colorWithHexRGBA:0x376FB1FF];
    Extra1Color = [NSColor colorWithHexRGBA:0x5C5C5CFF];
}

+ (NSColor *)comment
{
    return CommentColor;
}

+ (NSColor *)directive
{
    return DirectiveColor;
}

+ (NSColor *)string
{
    return StringColor;
}

+ (NSColor *)keyword
{
    return KeywordColor;
}

+ (NSColor *)extra1
{
    return Extra1Color;
}

@end
