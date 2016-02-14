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
}

- (void)setSelected:(BOOL)selected
{
    [super setSelected:selected];
    self.view.layer.backgroundColor = selected ? [NSColor grayColor].CGColor : [NSColor clearColor].CGColor;
}

@end
