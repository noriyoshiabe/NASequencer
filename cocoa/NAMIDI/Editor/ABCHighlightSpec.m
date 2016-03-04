//
//  ABCHighlightSpec.m
//  NAMIDI
//
//  Created by abechan on 3/5/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "ABCHighlightSpec.h"

static HighlightSpec *_spec;

@implementation ABCHighlightSpec

+ (void)initialize
{
    HighlightSpec *tune = [[HighlightSpec alloc] init];
    tune.regex = [NSRegularExpression regularExpressionWithPattern:@"(^|(?<=\n))X:.*?(\n\n|$)" options:NSRegularExpressionDotMatchesLineSeparators error:nil];
    tune.color = [HighlightColor tuneBody];
    
    HighlightSpec *tuneHeader = [[HighlightSpec alloc] init];
    tuneHeader.regex = [NSRegularExpression regularExpressionWithPattern:@"^X:.*?(K:.*?(?=\n|$)|\n(?![A-Z%]))" options:NSRegularExpressionDotMatchesLineSeparators error:nil];
    tuneHeader.color = [HighlightColor tuneHeader];

    HighlightSpec *freeText = [[HighlightSpec alloc] init];
    freeText.regex = [NSRegularExpression regularExpressionWithPattern:@".*" options:NSRegularExpressionDotMatchesLineSeparators error:nil];
    freeText.color = [HighlightColor freeText];
    
    HighlightSpec *directive = [[HighlightSpec alloc] init];
    directive.regex = [NSRegularExpression regularExpressionWithPattern:@"%.*$" options:NSRegularExpressionAnchorsMatchLines error:nil];
    directive.color = [HighlightColor directive];
    
    HighlightSpec *tuneBodydirective = [[HighlightSpec alloc] init];
    tuneBodydirective.regex = [NSRegularExpression regularExpressionWithPattern:@"%.*$" options:NSRegularExpressionAnchorsMatchLines error:nil];
    tuneBodydirective.color = [HighlightColor directive];
    
    HighlightSpec *information = [[HighlightSpec alloc] init];
    information.regex = [NSRegularExpression regularExpressionWithPattern:@"^[a-zA-Z]:.*$" options:NSRegularExpressionAnchorsMatchLines error:nil];
    information.color = [HighlightColor comment];
    
    HighlightSpec *decoration = [[HighlightSpec alloc] init];
    decoration.regex = [NSRegularExpression regularExpressionWithPattern:@"!.*?!" options:0 error:nil];
    decoration.color = [HighlightColor freeText];
    
    HighlightSpec *note = [[HighlightSpec alloc] init];
    note.regex = [NSRegularExpression regularExpressionWithPattern:@"(\\^\\^|\\^|=|_|__)?[a-gA-G](,|')*[0-9]*(\\/[0-9]*)*" options:0 error:nil];
    note.color = [HighlightColor note];
    
    HighlightSpec *rest = [[HighlightSpec alloc] init];
    rest.regex = [NSRegularExpression regularExpressionWithPattern:@"[xXzZ][0-9]*(\\/[0-9]*)*" options:0 error:nil];
    rest.color = [HighlightColor note];
    
    tune.next = freeText;
    tune.orverride = tuneHeader;
    
    freeText.orverride = directive;

    tuneHeader.orverride = directive;
    tuneHeader.next = tuneBodydirective;
    
    tuneBodydirective.next = information;
    information.next = decoration;
    decoration.next = note;
    note.next = rest;
    
    _spec = tune;
}

+ (HighlightSpec *)spec
{
    return _spec;
}

@end
