//
//  WelcomeWindow.m
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "WelcomeWindow.h"

@interface WelcomeWindow ()

@end

@implementation WelcomeWindow

- (void)awakeFromNib
{
    [super awakeFromNib];
    
    self.styleMask = NSTitledWindowMask | NSClosableWindowMask | NSFullSizeContentViewWindowMask;
    self.titleVisibility = NSWindowTitleHidden;
    self.titlebarAppearsTransparent = YES;
    
    [self standardWindowButton:NSWindowCloseButton].superview.hidden = YES;
}

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

- (BOOL)isMovableByWindowBackground
{
    return YES;
}

@end
