//
//  MainWindowController.m
//  NAMIDI
//
//  Created by abechan on 2/8/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MainWindowController.h"
#import "MainViewController.h"
#import "DetailViewController.h"

@interface MainWindowController ()
@property (strong, nonatomic) MainViewController *mainVC;
@property (strong, nonatomic) DetailViewController *detailVC;
@property (weak) IBOutlet NSToolbar *toolBar;
@end

@implementation MainWindowController

- (NSString *)windowNibName
{
    return @"MainWindowController";
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    self.windowFrameAutosaveName = @"MainWindowFrame";
    
    self.mainVC = [[MainViewController alloc] init];
    self.detailVC = [[DetailViewController alloc] init];
    
    self.mainVC.view.frame = self.window.contentView.bounds;
    self.contentViewController = self.mainVC;
    
    // TODO Title Bar Appearance
    //self.window.titleVisibility = NSWindowTitleHidden;
}

- (IBAction)nextButtonTapped:(id)sender
{
    self.detailVC.view.frame = self.window.contentView.bounds;
    self.contentViewController = self.detailVC;
}

- (IBAction)backButtonTapped:(id)sender
{
    self.mainVC.view.frame = self.window.contentView.bounds;
    self.contentViewController = self.mainVC;
}

@end
