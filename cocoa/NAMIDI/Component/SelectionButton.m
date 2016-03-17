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
    NSColor *_disableBorderColor;
    NSColor *_activeBackgroundColor;
    NSColor *_inactiveBackgroundColor;
    NSDictionary *_activeTextAttrs;
    NSDictionary *_inactiveTextAttrs;
}

@end

@implementation SelectionButton

- (void)awakeFromNib
{
    CGFloat fontSize = 14.0;
    
    if (0 < self.tag) {
        NSColor *baseColor = [Color channelColor:(int)self.tag];
        _activeBorderColor = [NSColor colorWithCalibratedHue:baseColor.hueComponent
                                                 saturation:baseColor.saturationComponent * 0.6
                                                 brightness:baseColor.brightnessComponent
                                                      alpha:baseColor.alphaComponent];
        _inactiveBorderColor = [NSColor colorWithCalibratedHue:baseColor.hueComponent
                                                    saturation:baseColor.saturationComponent * 0.6
                                                    brightness:baseColor.brightnessComponent * 0.8
                                                         alpha:baseColor.alphaComponent];
    }
    else {
        _activeBorderColor = [NSColor colorWith8bitRed:204 green:205 blue:212 alpha:255];
        _inactiveBorderColor = [NSColor colorWithCalibratedHue:_activeBorderColor.hueComponent
                                                    saturation:_activeBorderColor.saturationComponent
                                                    brightness:_activeBorderColor.brightnessComponent * 0.8
                                                         alpha:_activeBorderColor.alphaComponent];
    }
    
    _disableBorderColor = [Color gray];
    
    _activeBackgroundColor = [NSColor blackColor];
    _inactiveBackgroundColor = [Color darkGray];
    
    _activeTextAttrs = @{NSFontAttributeName:[NSFont boldSystemFontOfSize:fontSize], NSForegroundColorAttributeName: [NSColor whiteColor]};
    _inactiveTextAttrs = @{NSFontAttributeName:[NSFont boldSystemFontOfSize:fontSize], NSForegroundColorAttributeName: [Color gray]};
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSColor *borderColor = !self.enabled ? _disableBorderColor :
                            self.state == NSOnState ? _activeBorderColor : _inactiveBorderColor;
    
    NSColor *backgroundColor = self.state == NSOnState ? _activeBackgroundColor : _inactiveBackgroundColor;
    NSDictionary *textAttrs = self.state == NSOnState ? _activeTextAttrs : _inactiveTextAttrs;
    
    [borderColor setFill];
    NSBezierPath *border = [NSBezierPath bezierPathWithRoundedRect:self.bounds xRadius:5.0 yRadius:5.0];
    [border fill];
    
    [backgroundColor setFill];
    NSBezierPath *background = [NSBezierPath bezierPathWithRoundedRect:CGRectInset(self.bounds, 2.0, 2.0) xRadius:3.0 yRadius:3.0];
    [background fill];
    
    NSString *string = self.title;
    CGSize size = [string sizeWithAttributes:textAttrs];
    CGPoint point = CGPointMake(self.bounds.size.width / 2 - size.width / 2, self.bounds.size.height / 2 - size.height / 2 - 1.0);
    [string drawAtPoint:point withAttributes:textAttrs];
}

@end
