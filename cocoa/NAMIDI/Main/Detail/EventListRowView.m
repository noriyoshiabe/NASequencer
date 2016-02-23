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
static NSDictionary *RightTextAlignmentAttrs;

static CGRect LocationRect;
static CGRect TypeRect;
static CGRect ChannelRect;
static CGRect NoteRect;
static CGRect GatetimeRect;
static CGRect VelocityRect;
static CGRect OtherRect;

@implementation EventListRowView

+ (void)initialize
{
    NSMutableParagraphStyle *ps = [[NSMutableParagraphStyle alloc] init];
    ps.lineBreakMode = NSLineBreakByTruncatingTail;
    
    TextAttrs = @{NSFontAttributeName:[NSFont systemFontOfSize:10.0],
                  NSForegroundColorAttributeName: [Color gray],
                  NSParagraphStyleAttributeName: ps
                  };
    
    ps = [[NSMutableParagraphStyle alloc] init];
    ps.alignment = NSRightTextAlignment;
    
    RightTextAlignmentAttrs = @{NSFontAttributeName:[NSFont systemFontOfSize:10.0],
                                NSForegroundColorAttributeName: [Color gray],
                                NSParagraphStyleAttributeName: ps
                                };
    
    LocationRect = CGRectMake(8,   0,  65, 12);
    TypeRect     = CGRectMake(70,  0,  55, 12);
    ChannelRect  = CGRectMake(120, 0,  13, 12);
    NoteRect     = CGRectMake(138, 0,  33, 12);
    GatetimeRect = CGRectMake(160, 0,  26, 12);
    VelocityRect = CGRectMake(181, 0,  26, 12);
    OtherRect    = CGRectMake(211, 0, 191, 12);
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    
    [_location drawInRect:LocationRect withAttributes:TextAttrs];
    [_type drawInRect:TypeRect withAttributes:TextAttrs];
    [_channel drawInRect:ChannelRect withAttributes:RightTextAlignmentAttrs];
    [_note drawInRect:NoteRect withAttributes:TextAttrs];
    [_gatetime drawInRect:GatetimeRect withAttributes:RightTextAlignmentAttrs];
    [_velocity drawInRect:VelocityRect withAttributes:RightTextAlignmentAttrs];
    
    CGFloat width = self.frame.size.width - OtherRect.origin.x - 3;
    [_other drawInRect:CGRectMake(OtherRect.origin.x, OtherRect.origin.y, width, OtherRect.size.height) withAttributes:TextAttrs];
}

@end
