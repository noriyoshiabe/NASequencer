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
}

#pragma mark Toolbar Action

- (BOOL)validateToolbarItem:(NSToolbarItem *)theItem
{
    if ([theItem.itemIdentifier isEqualToString:@"back"]) {
        return NO;
    }
    else {
        return YES;
    }
}

- (IBAction)goBack:(id)sender
{
    NSLog(@"%s", __func__);
}

- (IBAction)rewind:(id)sender
{
    NSLog(@"%s", __func__);
}

- (IBAction)playPause:(NSToolbarItem *)sender
{
    NSLog(@"%s", __func__);
    sender.tag = !sender.tag;
    sender.image = [NSImage imageNamed:sender.tag ? @"play" : @"pause"];
}

- (IBAction)backward:(id)sender
{
    NSLog(@"%s", __func__);
}

- (IBAction)forward:(id)sender
{
    NSLog(@"%s", __func__);
}

- (IBAction)export:(id)sender
{
    NSLog(@"%s", __func__);
}

@end
