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
#import "ErrorWindowController.h"
#import "TrackSelection.h"

@interface MainWindowController () <TrackSelectionDelegate>
@property (weak) IBOutlet NSView *contentView;
@property (weak) IBOutlet LocationView *locationView;
@property (strong, nonatomic) MainViewController *mainVC;
@property (strong, nonatomic) DetailViewController *detailVC;
@property (strong, nonatomic) ErrorWindowController *errorWC;
@property (strong, nonatomic) TrackSelection *trackSelection;
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
    
    _trackSelection = [[TrackSelection alloc] init];
    _trackSelection.delegate = self;
    
    _mainVC.trackSelection = _trackSelection;
    _detailVC.trackSelection = _trackSelection;
    
    _locationView.player = [[PlayerRepresentation alloc]init];
    [self showMainView];
    
    [self showErrorWindow];
}

- (void)showErrorWindow
{
    if (!_errorWC) {
        _errorWC = [[ErrorWindowController alloc] init];
        CGRect frame = self.window.frame;
        frame.origin.x = frame.origin.x + frame.size.width / 2 - CGRectGetWidth(_errorWC.window.frame) / 2;
        frame.origin.y = frame.origin.y + frame.size.height / 2 - CGRectGetHeight(_errorWC.window.frame) / 2;
        frame.size = _errorWC.window.frame.size;
        [_errorWC.window setFrame:frame display:YES];
        
        [self.window addChildWindow:_errorWC.window ordered:NSWindowAbove];
    }
    
    [_errorWC showWindow:self];
}

- (void)showMainView
{
    [_detailVC.view removeFromSuperview];
    
    _mainVC.view.translatesAutoresizingMaskIntoConstraints = NO;
    [_contentView addSubview:_mainVC.view];
    [_contentView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:|[view]|" options:0 metrics:nil views:@{@"view": _mainVC.view}]];
    [_contentView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[view]|" options:0 metrics:nil views:@{@"view": _mainVC.view}]];
}

- (void)showDetailView
{
    [_mainVC.view removeFromSuperview];
    
    _detailVC.view.translatesAutoresizingMaskIntoConstraints = NO;
    [_contentView addSubview:_detailVC.view];
    [_contentView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:|[view]|" options:0 metrics:nil views:@{@"view": _detailVC.view}]];
    [_contentView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[view]|" options:0 metrics:nil views:@{@"view": _detailVC.view}]];
}

#pragma mark Toolbar Action

- (BOOL)validateToolbarItem:(NSToolbarItem *)theItem
{
    if ([theItem.itemIdentifier isEqualToString:@"back"]) {
        return nil != _detailVC.view.superview;
    }
    else {
        return YES;
    }
}

- (IBAction)goBack:(id)sender
{
    [self showMainView];
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

#pragma mark TrackSelectionDelegate

- (void)trackSelectionDidEnterSelection:(TrackSelection *)trackSelection
{
    [self showDetailView];
}

@end
