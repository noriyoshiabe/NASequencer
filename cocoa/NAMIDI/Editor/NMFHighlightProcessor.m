//
//  NMFHighlightProcessor.m
//  NAMIDI
//
//  Created by abechan on 3/5/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "NMFHighlightProcessor.h"

@interface NMFHighlightProcessor () {
    NSRegularExpression *_multiLineCommentRegex;
    NSRegularExpression *_lineCommentRegex;
}

@end

@implementation NMFHighlightProcessor

- (instancetype)init
{
    self = [super init];
    if (self) {
        _multiLineCommentRegex = [NSRegularExpression regularExpressionWithPattern:@"\\/\\*.*?\\*\\/" options:NSRegularExpressionDotMatchesLineSeparators error:nil];
        _lineCommentRegex = [NSRegularExpression regularExpressionWithPattern:@"\\/\\/.*$|(?<![_0-9a-zA-Z])--.*$|==.*$" options:NSRegularExpressionAnchorsMatchLines error:nil];
    }
    return self;
}

- (void)processTextStorage:(NSTextStorage *)textStorage
{
    NSRange range = NSMakeRange(0, textStorage.length);
    [textStorage removeAttribute:NSForegroundColorAttributeName range:range];
    [self processMultiLineComment:textStorage range:range];
}

- (void)processMultiLineComment:(NSTextStorage *)textStorage range:(NSRange)range
{
    __block NSUInteger prevLocation = range.location;
    
    [_multiLineCommentRegex enumerateMatchesInString:textStorage.string options:0 range:range usingBlock:^(NSTextCheckingResult * _Nullable result, NSMatchingFlags flags, BOOL * _Nonnull stop) {
        
        NSRange matchedRange = [result rangeAtIndex:0];
        [textStorage addAttribute:NSForegroundColorAttributeName value:[HighlightColor comment] range:matchedRange];
        
        if (prevLocation < matchedRange.location) {
            [self processLineComment:textStorage range:NSMakeRange(prevLocation, matchedRange.location - prevLocation)];
        }
        
        prevLocation = matchedRange.location + matchedRange.length;
    }];
    
    if (prevLocation < range.length) {
        [self processLineComment:textStorage range:NSMakeRange(prevLocation, range.length - prevLocation)];
    }
}

- (void)processLineComment:(NSTextStorage *)textStorage range:(NSRange)range
{
    __block NSUInteger prevLocation = range.location;
    
    __Trace__
    
    [_lineCommentRegex enumerateMatchesInString:textStorage.string options:0 range:range usingBlock:^(NSTextCheckingResult * _Nullable result, NSMatchingFlags flags, BOOL * _Nonnull stop) {
        
        NSRange matchedRange = [result rangeAtIndex:0];
        [textStorage addAttribute:NSForegroundColorAttributeName value:[HighlightColor comment] range:matchedRange];
        
        if (prevLocation < matchedRange.location) {
        }
        
        prevLocation = matchedRange.location + matchedRange.length;
    }];
    
    if (prevLocation < range.length) {
    }
}

@end
