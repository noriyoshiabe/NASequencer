//
//  ListenToExampleRowView.m
//  NAMIDI
//
//  Created by abechan on 3/4/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "ListenToExampleRowView.h"
#import "Color.h"

@interface ListenToExampleRowView ()
@property (strong) IBOutlet NSView *contentView;

@end

@implementation ListenToExampleRowView

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    if (!_contentView) {
        [[NSBundle mainBundle] loadNibNamed:@"ListenToExampleRowView" owner:self topLevelObjects:nil];
        [self addSubview:_contentView];
    }
}

- (void)drawSelectionInRect:(NSRect)dirtyRect
{
    NSColor *pink = [Color pink];
    [[NSColor colorWithRed:pink.redComponent green:pink.greenComponent blue:pink.greenComponent alpha:0.5] set];
    [[NSBezierPath bezierPathWithRect:self.bounds] fill];
}

@end
