//
//  ColorButton.h
//  NAMIDI
//
//  Created by abechan on 3/4/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface ColorButton : NSButton
@property (copy, nonatomic) NSColor *activeBorderColor;
@property (copy, nonatomic) NSColor *inactiveBorderColor;
@property (copy, nonatomic) NSColor *disableBorderColor;
@property (copy, nonatomic) NSColor *activeBackgroundColor;
@property (copy, nonatomic) NSColor *inactiveBackgroundColor;
@property (copy, nonatomic) NSColor *disableBackgroundColor;
@property (copy, nonatomic) NSColor *activeTextColor;
@property (copy, nonatomic) NSColor *inactiveTextColor;
@property (copy, nonatomic) NSColor *disableTextColor;
@property (nonatomic) CGFloat activeFontWeight;
@property (nonatomic) CGFloat borderWidth;
@property (nonatomic) CGFloat borderRadius;
@end
