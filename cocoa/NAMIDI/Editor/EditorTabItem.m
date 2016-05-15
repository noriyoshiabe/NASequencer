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
}

- (void)setActive:(BOOL)active
{
    _active = active;
    
    self.view.layer.backgroundColor = active ? [Color darkGray].CGColor : [NSColor lightGrayColor].CGColor;
    self.textField.textColor = active ? [NSColor whiteColor] : [NSColor blackColor];
    self.textField.font = active ? [NSFont boldSystemFontOfSize:self.textField.font.pointSize] : [NSFont systemFontOfSize:self.textField.font.pointSize];
}

- (IBAction)closeButtonPressed:(id)sender
{
    [_delegate tabItemDidPressCloseButton:self];
}

@end
