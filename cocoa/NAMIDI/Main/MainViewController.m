//
//  MainViewController.m
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MainViewController.h"

@interface FlipImageView : NSImageView
@end

@interface MainViewController ()
@property (weak) IBOutlet NSScrollView *mixerView;
@property (weak) IBOutlet NSScrollView *conductorView;
@property (weak) IBOutlet NSScrollView *trackView;
@end

@implementation MainViewController

- (NSString *)nibName
{
    return @"MainViewController";
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    NSImage *image = [NSApp applicationIconImage];
    NSImageView *imageView;
    
    imageView = [[FlipImageView alloc] initWithFrame:CGRectMake(0, 0, 250, 1000)];
    imageView.image = image;
    _mixerView.documentView = imageView;
    
    imageView = [[FlipImageView alloc] initWithFrame:CGRectMake(0, 0, 1000, 56)];
    imageView.image = image;
    _conductorView.documentView = imageView;
    
    imageView = [[FlipImageView alloc] initWithFrame:CGRectMake(0, 0, 1000, 1000)];
    imageView.image = image;
    _trackView.documentView = imageView;
    
    _mixerView.contentView.postsBoundsChangedNotifications = YES;
    _conductorView.contentView.postsBoundsChangedNotifications = YES;
    _trackView.contentView.postsBoundsChangedNotifications = YES;
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(scrollViewContentBoundsDidChange:)
                                                 name:NSViewBoundsDidChangeNotification
                                               object:_mixerView.contentView];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(scrollViewContentBoundsDidChange:)
                                                 name:NSViewBoundsDidChangeNotification
                                               object:_conductorView.contentView];
    
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
    NSPoint changedBoundsOrigin = [changedContentView documentVisibleRect].origin;
    
    if (changedContentView == _mixerView.contentView) {
        NSPoint offset = _trackView.contentView.bounds.origin;
        if (offset.y != changedBoundsOrigin.y) {
            offset.y = changedBoundsOrigin.y;
            [_trackView.contentView scrollToPoint:offset];
            [_trackView reflectScrolledClipView:_trackView.contentView];
        }
    }
    else if (changedContentView == _conductorView.contentView) {
        NSPoint offset = _trackView.contentView.bounds.origin;
        if (offset.x != changedBoundsOrigin.x) {
            offset.x = changedBoundsOrigin.x;
            [_trackView.contentView scrollToPoint:offset];
            [_trackView reflectScrolledClipView:_trackView.contentView];
        }
    }
    else if (changedContentView == _trackView.contentView) {
        NSPoint offset;
        
        offset = _conductorView.contentView.bounds.origin;
        if (offset.x != changedBoundsOrigin.x) {
            offset.x = changedBoundsOrigin.x;
            [_conductorView.contentView scrollToPoint:offset];
            [_conductorView reflectScrolledClipView:_conductorView.contentView];
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

#pragma mark Flip

@implementation FlipImageView

- (BOOL)isFlipped
{
    return YES;
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
