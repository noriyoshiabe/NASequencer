//
//  PianoRollConductorLabelView.m
//  NAMIDI
//
//  Created by abechan on 2/23/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PianoRollConductorLabelView.h"
#import "Color.h"

#define LABEL_X 3.0

@interface PianoRollConductorLabelView () {
    CGColorRef _lineColor;
    NSDictionary *_textAttrs;
}

@end

@implementation PianoRollConductorLabelView

- (void)awakeFromNib
{
    _lineColor = [Color gray].CGColor;
    _textAttrs = @{NSFontAttributeName:[NSFont systemFontOfSize:10.0], NSForegroundColorAttributeName: [Color gray]};
}

- (BOOL)isFlipped
{
    return YES;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    CGContextRef ctx = [NSGraphicsContext currentContext].graphicsPort;
    
    [self drawLine:dirtyRect context:ctx];
    [self drawText:dirtyRect context:ctx];
}

- (void)drawLine:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    CGContextSetLineWidth(ctx, 0.5);
    CGContextSetStrokeColorWithColor(ctx, _lineColor);
    
    CGFloat third = floor(self.bounds.size.height / 3);
    
    for (int i = 1; i <= 2; ++i) {
        CGFloat y = third * i + 0.5;
        CGContextMoveToPoint(ctx, 0, y);
        CGContextAddLineToPoint(ctx, self.bounds.size.width, y);
        CGContextStrokePath(ctx);
    }
    
    CGContextClipToRect(ctx, dirtyRect);
    
    CGContextRestoreGState(ctx);
}

- (void)drawText:(NSRect)dirtyRect context:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);
    
    const NSArray<NSString *> *labels = @[
                                          @"Time Sign",
                                          @"Tempo",
                                          @"Marker",
                                          ];
    
    CGFloat sixth = floor(self.bounds.size.height / 6);
    for (int i = 0; i < 3; ++i) {
        CGFloat height = [labels[i] sizeWithAttributes:_textAttrs].height;
        CGFloat y = sixth * i * 2 + sixth - height / 2;
        
        [labels[i] drawAtPoint:CGPointMake(LABEL_X, y) withAttributes:_textAttrs];
    }
    
    CGContextRestoreGState(ctx);
}

@end
