//
//  SelectionButton.m
//  NAMIDI
//
//  Created by abechan on 2/21/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "SelectionButton.h"
#import "Color.h"

@interface SelectionButton () {
    NSColor *_activeBorderColor;
    NSColor *_inactiveBorderColor;
    NSColor *_activeBackgroundColor;
    NSColor *_inactiveBackgroundColor;
    NSDictionary *_activeTextAttrs;
    NSDictionary *_inactiveTextAttrs;
}

@end

@implementation SelectionButton

- (void)awakeFromNib
{
    CGFloat fontSize;
    
    if (0 < self.tag) {
        NSColor *baseColor = [Color channelColor:(int)self.tag];
        _activeBorderColor = [NSColor colorWithCalibratedHue:baseColor.hueComponent
                                                 saturation:baseColor.saturationComponent * 0.6
                                                 brightness:baseColor.brightnessComponent
                                                      alpha:baseColor.alphaComponent];
        fontSize = 9.0;
    }
    else {
        _activeBorderColor = [NSColor colorWith8bitRed:204 green:205 blue:212 alpha:255];
        fontSize = 8.0;
    }
    
    _inactiveBorderColor = [Color gray];
    _activeBackgroundColor = [NSColor blackColor];
    _inactiveBackgroundColor = [Color darkGray];
    
    _activeTextAttrs = @{NSFontAttributeName:[NSFont boldSystemFontOfSize:fontSize], NSForegroundColorAttributeName: [NSColor whiteColor]};
    _inactiveTextAttrs = @{NSFontAttributeName:[NSFont boldSystemFontOfSize:fontSize], NSForegroundColorAttributeName: [Color gray]};
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSColor *borderColor = self.state == NSOnState ? _activeBorderColor : _inactiveBorderColor;
    NSColor *backgroundColor = self.state == NSOnState ? _activeBackgroundColor : _inactiveBackgroundColor;
    NSDictionary *textAttrs = self.state == NSOnState ? _activeTextAttrs : _inactiveTextAttrs;
    
    [borderColor setFill];
    NSBezierPath *border = [NSBezierPath bezierPathWithRoundedRect:self.bounds xRadius:5.0 yRadius:5.0];
    [border fill];
    
    [backgroundColor setFill];
    NSBezierPath *background = [NSBezierPath bezierPathWithRoundedRect:CGRectInset(self.bounds, 1.0, 1.0) xRadius:4.0 yRadius:4.0];
    [background fill];
    
    NSString *string = self.title;
    CGSize size = [string sizeWithAttributes:textAttrs];
    CGPoint point = CGPointMake(self.bounds.size.width / 2 - size.width / 2, self.bounds.size.height / 2 - size.height / 2 - 1.0);
    [string drawAtPoint:point withAttributes:textAttrs];
}

@end