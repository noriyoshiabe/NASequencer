//
//  ColorButton.m
//  NAMIDI
//
//  Created by abechan on 3/4/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "ColorButton.h"

@implementation ColorButton

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    _borderWidth = _borderWidth ? _borderWidth : 1.0;
    _borderRadius = _borderRadius ? _borderRadius : 3.0;
    
    _activeBorderColor = _activeBorderColor ? _activeBorderColor : [NSColor grayColor];
    _inactiveBorderColor = _inactiveBorderColor ? _inactiveBorderColor : _activeBorderColor;
    _disableBorderColor = _disableBorderColor ? _disableBorderColor : _inactiveBorderColor;
    _activeBackgroundColor = _activeBackgroundColor ? _activeBackgroundColor : [NSColor whiteColor];
    _inactiveBackgroundColor = _inactiveBackgroundColor ? _inactiveBackgroundColor : _activeBackgroundColor;
    _disableBackgroundColor = _disableBackgroundColor ? _disableBackgroundColor : _inactiveBackgroundColor;
    _activeTextColor = _activeTextColor ? _activeTextColor : [NSColor blackColor];
    _inactiveTextColor = _inactiveTextColor ? _inactiveTextColor : _activeTextColor;
    _disableTextColor = _disableTextColor ? _disableTextColor : _inactiveTextColor;
}

- (void)drawRect:(NSRect)dirtyRect
{
    NSColor *borderColor = !self.enabled ? _disableBorderColor :
    self.state == NSOnState ? _activeBorderColor : _inactiveBorderColor;
    NSColor *backgroundColor = self.state == NSOnState ? _activeBackgroundColor : _inactiveBackgroundColor;
    NSColor *textColor = !self.enabled ? _disableTextColor : self.state == NSOnState ? _activeTextColor : _inactiveTextColor;
    
    [NSBezierPath setDefaultLineWidth:_borderWidth];
    [borderColor setFill];
    NSBezierPath *border = [NSBezierPath bezierPathWithRoundedRect:self.bounds xRadius:_borderRadius yRadius:_borderRadius];
    [border fill];
    
    [backgroundColor setFill];
    NSBezierPath *background = [NSBezierPath bezierPathWithRoundedRect:CGRectInset(self.bounds, _borderWidth, _borderWidth) xRadius:_borderRadius yRadius:_borderRadius];
    [background fill];
    
    if (textColor) {
        NSFont *font = _activeFontWeight && self.enabled && self.state == NSOnState ? [NSFont systemFontOfSize:self.font.pointSize weight:_activeFontWeight] : self.font;
        NSDictionary *textAttrs = @{NSFontAttributeName:font, NSForegroundColorAttributeName: textColor};
        NSString *string = self.title;
        CGSize size = [string sizeWithAttributes:textAttrs];
        CGPoint point = CGPointMake(self.bounds.size.width / 2 - size.width / 2, self.bounds.size.height / 2 - size.height / 2 - 1.0);
        [string drawAtPoint:point withAttributes:textAttrs];
    }
}

@end
