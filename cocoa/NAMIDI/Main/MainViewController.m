//
//  MainViewController.m
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MainViewController.h"
#import "ConductorViewController.h"
#import "MeasureViewController.h"
#import "MixerViewController.h"
#import "TrackViewController.h"
#import "Color.h"

@interface MainViewController ()
@property (weak) IBOutlet NSView *conductorView;
@property (weak) IBOutlet NSScrollView *mixerView;
@property (weak) IBOutlet NSScrollView *measureView;
@property (weak) IBOutlet NSScrollView *trackView;
@property (strong, nonatomic) ConductorViewController *conductorVC;
@property (strong, nonatomic) MeasureViewController *measureVC;
@property (strong, nonatomic) MixerViewController *mixerVC;
@property (strong, nonatomic) TrackViewController *trackVC;
@property (weak) IBOutlet NSView *horizontalLine;
@property (weak) IBOutlet NSView *verticalLine;
@end

@implementation MainViewController

- (NSString *)nibName
{
    return @"MainViewController";
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _conductorView.layer.backgroundColor = [Color darkGray].CGColor;
    _mixerView.backgroundColor = [Color darkGray];
    _measureView.backgroundColor = [Color darkGray];
    
    _horizontalLine.layer.backgroundColor = [Color gray].CGColor;
    _verticalLine.layer.backgroundColor = [Color gray].CGColor;
    
    self.conductorVC = [[ConductorViewController alloc] init];
    self.measureVC = [[MeasureViewController alloc] init];
    self.mixerVC = [[MixerViewController alloc] init];
    self.trackVC = [[TrackViewController alloc] init];
    
    _conductorVC.view.frame = _conductorView.bounds;
    
    [_conductorView addSubview:_conductorVC.view];
    _measureView.documentView = _measureVC.view;
    _mixerView.documentView = _mixerVC.view;
    _trackView.documentView = _trackVC.view;
    
    _mixerView.contentView.postsBoundsChangedNotifications = YES;
    _measureView.contentView.postsBoundsChangedNotifications = YES;
    _trackView.contentView.postsBoundsChangedNotifications = YES;
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(scrollViewContentBoundsDidChange:)
                                                 name:NSViewBoundsDidChangeNotification
                                               object:_mixerView.contentView];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(scrollViewContentBoundsDidChange:)
                                                 name:NSViewBoundsDidChangeNotification
                                               object:_measureView.contentView];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(scrollViewContentBoundsDidChange:)
                                                 name:NSViewBoundsDidChangeNotification
                                               object:_trackView.contentView];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)scrollViewContentBoundsDidChange:(NSNotification *)notification
{
    NSClipView *changedContentView = [notification object];
    NSPoint changedBoundsOrigin = changedContentView.bounds.origin;
    
    if (changedContentView == _mixerView.contentView) {
        NSPoint offset = _trackView.contentView.bounds.origin;
        if (offset.y != changedBoundsOrigin.y) {
            offset.y = changedBoundsOrigin.y;
            [_trackView.contentView scrollToPoint:offset];
            [_trackView reflectScrolledClipView:_trackView.contentView];
        }
    }
    else if (changedContentView == _measureView.contentView) {
        NSPoint offset = _trackView.contentView.bounds.origin;
        if (offset.x != changedBoundsOrigin.x) {
            offset.x = changedBoundsOrigin.x;
            [_trackView.contentView scrollToPoint:offset];
            [_trackView reflectScrolledClipView:_trackView.contentView];
        }
    }
    else if (changedContentView == _trackView.contentView) {
        NSPoint offset;
        
        offset = _measureView.contentView.bounds.origin;
        if (offset.x != changedBoundsOrigin.x) {
            offset.x = changedBoundsOrigin.x;
            [_measureView.contentView scrollToPoint:offset];
            [_measureView reflectScrolledClipView:_measureView.contentView];
        }
        
        if (offset.y != changedBoundsOrigin.y) {
            offset = _mixerView.contentView.bounds.origin;
            offset.y = changedBoundsOrigin.y;
            [_mixerView.contentView scrollToPoint:offset];
            [_mixerView reflectScrolledClipView:_mixerView.contentView];
        }
    }
}

@end

#pragma mark Hide scroll bar

@interface MainWindowScroller : NSScroller
@end

@implementation MainWindowScroller

+ (BOOL)isCompatibleWithOverlayScrollers
{
    return YES;
}

- (void)setHidden:(BOOL)flag
{
    [super setHidden:YES];
}

@end
