//
//  MainWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/8/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MainWindowController.h"
#import "MainViewController.h"

@interface MainWindowController ()
@property (strong, nonatomic) MainViewController *mainVC;
@end

@implementation MainWindowController

- (void)awakeFromNib
{
    [super awakeFromNib];
    self.window.frameAutosaveName = @"MainWindowFrame";
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    self.mainVC = [[MainViewController alloc] init];
    self.mainVC.view.frame = self.window.contentView.bounds;
    self.contentViewController = self.mainVC;
}

@end
