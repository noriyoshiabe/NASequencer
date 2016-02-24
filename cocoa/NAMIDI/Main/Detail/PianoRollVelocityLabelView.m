//
//  PianoRollVelocityLabelView.m
//  NAMIDI
//
//  Created by abechan on 2/24/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PianoRollVelocityLabelView.h"
#import "Color.h"

#define LABEL_X 3.0
#define LABEL_Y 1.0

@interface PianoRollVelocityLabelView () {
    NSDictionary *_textAttrs;
}

@end

@implementation PianoRollVelocityLabelView

- (void)awakeFromNib
{
    _textAttrs = @{NSFontAttributeName:[NSFont systemFontOfSize:10.0], NSForegroundColorAttributeName:[Color gray]};
}

- (BOOL)isFlipped
{
    return YES;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    [@"Velocity" drawAtPoint:CGPointMake(LABEL_X, LABEL_Y) withAttributes:_textAttrs];
}

@end
