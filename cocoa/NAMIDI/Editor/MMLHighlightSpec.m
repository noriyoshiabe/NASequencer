//
//  MMLHighlightSpec.m
//  NAMIDI
//
//  Created by abechan on 3/5/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MMLHighlightSpec.h"

static HighlightSpec *_spec;

@implementation MMLHighlightSpec

+ (void)initialize
{
    HighlightSpec *multiLineComment = [[HighlightSpec alloc] init];
    multiLineComment.regex = [NSRegularExpression regularExpressionWithPattern:@"\\/\\*.*?\\*\\/" options:NSRegularExpressionDotMatchesLineSeparators error:nil];
    multiLineComment.color = [HighlightColor comment];
    
    HighlightSpec *lineComment = [[HighlightSpec alloc] init];
    lineComment.regex = [NSRegularExpression regularExpressionWithPattern:@"\\/\\/.*$" options:NSRegularExpressionAnchorsMatchLines error:nil];
    lineComment.color = [HighlightColor comment];
    
    HighlightSpec *directive = [[HighlightSpec alloc] init];
    directive.regex = [NSRegularExpression regularExpressionWithPattern:@"^\\s*#\\s*[a-zA-Z]+.*$" options:NSRegularExpressionAnchorsMatchLines error:nil];
    directive.color = [HighlightColor directive];
    
    HighlightSpec *string = [[HighlightSpec alloc] init];
    string.regex = [NSRegularExpression regularExpressionWithPattern:@"\"[^\"]*\"|'[^']*'" options:NSRegularExpressionDotMatchesLineSeparators error:nil];
    string.color = [HighlightColor string];
    
    HighlightSpec *keyword = [[HighlightSpec alloc] init];
    keyword.regex = [NSRegularExpression regularExpressionWithPattern:
#define KEYWORD(keyword) @"" keyword @""
                     KEYWORD(@"@ch\\s*[0-9]*") @"|"
                     KEYWORD(@"@sy") @"|"
                     KEYWORD(@"@bs\\s*[0-9]*") @"|"
                     KEYWORD(@"@pc\\s*[0-9]*") @"|"
                     KEYWORD(@"@vl\\s*[0-9]*") @"|"
                     KEYWORD(@"@cs\\s*[0-9]*") @"|"
                     KEYWORD(@"@rv\\s*[0-9]*") @"|"
                     KEYWORD(@"@x\\s*[0-9]*") @"|"
                     KEYWORD(@"@?pb\\s*[0-9\\-]*") @"|"
                     KEYWORD(@"@ps\\s*[0-9]*") @"|"
                     KEYWORD(@"@p\\s*[0-9]*") @"|"
                     KEYWORD(@"@d\\s*[0-9\\-]*") @"|"
                     KEYWORD(@"t\\s*[0-9\\.]*") @"|"
                     KEYWORD(@"bt\\s*[0-9\\.]*,\\s*[0-9]*") @"|"
                     KEYWORD(@"o\\s*[0-9]*") @"|"
                     KEYWORD(@"@?ns\\s*[0-9\\-]*") @"|"
                     KEYWORD(@"l\\s*[0-9]*") @"|"
                     KEYWORD(@"@?q\\s*[0-9]*") @"|"
                     KEYWORD(@"@?v\\s*[0-9]*") @"|"
                     KEYWORD(@"p|x") @"|"
#undef KEYWORD
                                                              options:NSRegularExpressionCaseInsensitive error:nil];
    keyword.color = [HighlightColor keyword];
    
    multiLineComment.next = lineComment;
    lineComment.next = directive;
    directive.next = string;
    string.next = keyword;
    
    _spec = multiLineComment;
}

+ (HighlightSpec *)spec
{
    return _spec;
}

@end
