//
//  EditorTabItem.m
//  NAMIDI
//
//  Created by abechan on 2/14/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "EditorTabItem.h"

@interface EditorTabItem ()

@end

@implementation EditorTabItem

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.view.wantsLayer = YES;
    self.selected = NO;
}

- (void)setSelected:(BOOL)selected
{
    [super setSelected:selected];
    self.view.layer.backgroundColor = selected ? [NSColor darkGrayColor].CGColor : [NSColor lightGrayColor].CGColor;
    self.textField.textColor = selected ? [NSColor whiteColor] : [NSColor blackColor];
    self.textField.font = selected ? [NSFont boldSystemFontOfSize:self.textField.font.pointSize] : [NSFont systemFontOfSize:self.textField.font.pointSize];
}

@end
