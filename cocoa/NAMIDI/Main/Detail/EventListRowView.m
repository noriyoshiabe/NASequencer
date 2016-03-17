//
//  EventListRowView.m
//  NAMIDI
//
//  Created by abechan on 2/24/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "EventListRowView.h"
#import "Color.h"

static NSDictionary *TextAttrs;

@interface EventListRowView () {
    NSTextField *_contentLabel;
}
@end

@implementation EventListRowView

+ (void)initialize
{
    NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
    ps.lineBreakMode = NSLineBreakByTruncatingTail;
    
    TextAttrs = @{NSFontAttributeName:[NSFont userFixedPitchFontOfSize:12],
                  NSForegroundColorAttributeName: [NSColor blackColor],
                  NSParagraphStyleAttributeName: ps
                  };
}

- (void)awakeFromNib
{
    _contentLabel = self.subviews[0];
}

- (void)setContent:(NSString *)content
{
    _contentLabel.stringValue = content;
}

@end
