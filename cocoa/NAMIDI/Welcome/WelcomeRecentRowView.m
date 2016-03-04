//
//  WelcomeRecentRowView.m
//  NAMIDI
//
//  Created by abechan on 3/4/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "WelcomeRecentRowView.h"
#import "Color.h"

@interface WelcomeRecentRowView ()
@property (strong) IBOutlet NSView *contentView;
@end

@implementation WelcomeRecentRowView

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    if (!_contentView) {
        [[NSBundle mainBundle] loadNibNamed:@"WelcomeRecentRowView" owner:self topLevelObjects:nil];
        [self addSubview:_contentView];
    }
}

- (void)drawSelectionInRect:(NSRect)dirtyRect
{
    [[Color pink] set];
    [[NSBezierPath bezierPathWithRect:self.bounds] fill];
}

@end
