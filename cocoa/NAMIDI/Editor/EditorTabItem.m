//
//  EditorTabItem.m
//  NAMIDI
//
//  Created by abechan on 2/14/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "EditorTabItem.h"
#import "Color.h"

@interface EditorTabItem ()

@end

@implementation EditorTabItem

- (void)awakeFromNib
{
    [super awakeFromNib];
    self.view.wantsLayer = YES;
    self.active = NO;
    
    [[NSDistributedNotificationCenter defaultCenter] addObserver:self selector:@selector(didChangeEffectiveAppearance:) name:@"AppleInterfaceThemeChangedNotification" object:nil];
}

- (void)dealloc
{
    [[NSDistributedNotificationCenter defaultCenter] removeObserver:self];
}

- (void)setActive:(BOOL)active
{
    _active = active;
    
    [self setColor];
    
    self.textField.font = active ? [NSFont boldSystemFontOfSize:self.textField.font.pointSize] : [NSFont systemFontOfSize:self.textField.font.pointSize];
}

- (void)setColor
{
    if (NSColor.isDarkMode) {
        self.view.layer.backgroundColor = _active ? [Color darkGray].CGColor : [Color ultraDarkGray].CGColor;
        self.textField.textColor = _active ? [NSColor whiteColor] : [NSColor grayColor];
    }
    else {
        self.view.layer.backgroundColor = _active ? [Color darkGray].CGColor : [NSColor lightGrayColor].CGColor;
        self.textField.textColor = _active ? [NSColor whiteColor] : [NSColor blackColor];
    }
}

- (void)didChangeEffectiveAppearance:(NSNotification *)notification
{
    dispatch_async(dispatch_get_main_queue(), ^{
       [self setColor];
    });
}

- (IBAction)closeButtonPressed:(id)sender
{
    [_delegate tabItemDidPressCloseButton:self];
}

@end
