//
//  NMFHighlightSpec.m
//  NAMIDI
//
//  Created by abechan on 3/5/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "NMFHighlightSpec.h"

static HighlightSpec *_spec;

@implementation NMFHighlightSpec

+ (void)initialize
{
    HighlightSpec *multiLineComment = [[HighlightSpec alloc] init];
    multiLineComment.regex = [NSRegularExpression regularExpressionWithPattern:@"\\/\\*.*?\\*\\/" options:NSRegularExpressionDotMatchesLineSeparators error:nil];
    multiLineComment.color = [HighlightColor comment];
    
    HighlightSpec *lineComment = [[HighlightSpec alloc] init];
    lineComment.regex = [NSRegularExpression regularExpressionWithPattern:@"\\/\\/.*$|(?<![_0-9a-zA-Z\\-])--.*$|==.*$" options:NSRegularExpressionAnchorsMatchLines error:nil];
    lineComment.color = [HighlightColor comment];
    
    HighlightSpec *directive = [[HighlightSpec alloc] init];
    directive.regex = [NSRegularExpression regularExpressionWithPattern:@"^\\s*#\\s*[a-zA-Z]+.*$" options:NSRegularExpressionAnchorsMatchLines error:nil];
    directive.color = [HighlightColor directive];
    
    HighlightSpec *string = [[HighlightSpec alloc] init];
    string.regex = [NSRegularExpression regularExpressionWithPattern:@"\"[^\"]*\"|'[^']*'" options:NSRegularExpressionDotMatchesLineSeparators error:nil];
    string.color = [HighlightColor string];

    HighlightSpec *keyword = [[HighlightSpec alloc] init];
    keyword.regex = [NSRegularExpression regularExpressionWithPattern:
#define KEYWORD(keyword) @"(?<!-)\\b" keyword @"\\b(?!-)"
                     KEYWORD(@"TITLE") @"|"
                     KEYWORD(@"COPYRIGHT") @"|"
                     KEYWORD(@"RESOLUTION") @"|"
                     KEYWORD(@"TEMPO") @"|"
                     KEYWORD(@"TIME") @"|"
                     KEYWORD(@"MARKER") @"|"
                     KEYWORD(@"CHANNEL") @"|"
                     KEYWORD(@"SYNTH") @"|"
                     KEYWORD(@"VOICE") @"|"
                     KEYWORD(@"CHORUS") @"|"
                     KEYWORD(@"REVERB") @"|"
                     KEYWORD(@"VOLUME") @"|"
                     KEYWORD(@"VELOCITY") @"|"
                     KEYWORD(@"EXPRESSION") @"|"
                     KEYWORD(@"TRANSPOSE") @"|"
                     KEYWORD(@"PAN") @"|"
                     KEYWORD(@"GATETIME") @"|"
                     KEYWORD(@"PATTERN") @"|"
                     KEYWORD(@"END") @"|"
                     KEYWORD(@"EXPAND") @"|"
                     KEYWORD(@"DETUNE") @"|"
                     KEYWORD(@"KEY")
#undef KEYWORD
                                                              options:NSRegularExpressionCaseInsensitive error:nil];
    keyword.color = [HighlightColor keyword];
    
    HighlightSpec *step = [[HighlightSpec alloc] init];
    step.regex = [NSRegularExpression regularExpressionWithPattern:@"[0-9]+\\s*:" options:0 error:nil];
    step.color = [HighlightColor step];
    
    multiLineComment.next = lineComment;
    lineComment.next = directive;
    directive.next = string;
    string.next = keyword;
    keyword.next = step;
    
    _spec = multiLineComment;
}

+ (HighlightSpec *)spec
{
    return _spec;
}

@end
