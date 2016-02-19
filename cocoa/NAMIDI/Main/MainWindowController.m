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
#import "LocationView.h"

@interface MainWindowController ()
@property (weak) IBOutlet NSView *contentView;
@property (weak) IBOutlet LocationView *locationView;
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
    
    self.window.contentView.wantsLayer = YES;
    self.window.contentView.layer.masksToBounds = YES;
    
    _mainVC = [[MainViewController alloc] init];
    _detailVC = [[DetailViewController alloc] init];
    
    _mainVC.view.translatesAutoresizingMaskIntoConstraints = NO;
    [_contentView addSubview:_mainVC.view];
    [_contentView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:|[view]|" options:0 metrics:nil views:@{@"view": _mainVC.view}]];
    [_contentView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[view]|" options:0 metrics:nil views:@{@"view": _mainVC.view}]];
    
    _locationView.player = [[PlayerRepresentation alloc]init];
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
