//
//  PianoRollViewController.m
//  NAMIDI
//
//  Created by abechan on 2/22/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PianoRollViewController.h"
#import "MeasureViewController.h"
#import "PlayLineViewController.h"
#import "SynchronizedScrollView.h"
#import "Color.h"

#define VELOCITY_VIEW_HEIGHT_MAX 160
#define VELOCITY_VIEW_HEIGHT_MIN 40

@interface PianoRollViewController () <NSSplitViewDelegate>
@property (weak) IBOutlet NSSplitView *horizontalSplitView;
@property (weak) IBOutlet NSView *emptyHeaderView;
@property (weak) IBOutlet NSView *conductorLabelView;
@property (weak) IBOutlet NSView *velocityLabelView;
@property (weak) IBOutlet SynchronizedScrollView *measureView;
@property (weak) IBOutlet SynchronizedScrollView *conductorView;
@property (weak) IBOutlet SynchronizedScrollView *keyboardView;
@property (weak) IBOutlet SynchronizedScrollView *pianoRollView;
@property (weak) IBOutlet SynchronizedScrollView *velocityView;
@property (weak) IBOutlet SynchronizedScrollView *playLineView;
@property (weak) IBOutlet NSView *measureBottomLineView;
@property (weak) IBOutlet NSView *conductorBottomLineView;
@property (weak) IBOutlet NSView *pianoRollVerticalLineView;
@property (weak) IBOutlet NSView *verocityVerticalLineView;

@property (strong, nonatomic) MeasureViewController *measureVC;
@property (strong, nonatomic) PlayLineViewController *playLineVC;
@end

@implementation PianoRollViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _horizontalSplitView.delegate = self;
    
    _emptyHeaderView.wantsLayer = YES;
    _emptyHeaderView.layer.backgroundColor = [Color ultraLightGray].CGColor;
    _conductorLabelView.wantsLayer = YES;
    _conductorLabelView.layer.backgroundColor = [NSColor whiteColor].CGColor;
    _velocityLabelView.wantsLayer = YES;
    _velocityLabelView.layer.backgroundColor = [NSColor whiteColor].CGColor;
    
    _measureView.backgroundColor = [Color ultraLightGray];
    _conductorView.backgroundColor = [NSColor whiteColor];
    _keyboardView.backgroundColor = [NSColor whiteColor];
    _pianoRollView.backgroundColor = [NSColor whiteColor];
    _velocityView.backgroundColor = [NSColor whiteColor];
    
    _measureBottomLineView.wantsLayer = YES;
    _measureBottomLineView.layer.backgroundColor = [Color gray].CGColor;
    _conductorBottomLineView.wantsLayer = YES;
    _conductorBottomLineView.layer.backgroundColor = [Color gray].CGColor;
    _pianoRollVerticalLineView.wantsLayer = YES;
    _pianoRollVerticalLineView.layer.backgroundColor = [Color gray].CGColor;
    _verocityVerticalLineView.wantsLayer = YES;
    _verocityVerticalLineView.layer.backgroundColor = [Color gray].CGColor;
    
    _playLineView.userInteractionEnabled = NO;
    
    _measureVC = [[MeasureViewController alloc] init];
    _measureVC.lineColor = [NSColor blackColor];
    _measureVC.measureNoColor = [NSColor blackColor];
    _measureVC.measureNoY = 16.5;
    _measureVC.needBottomLine = NO;
    
    _playLineVC = [[PlayLineViewController alloc] init];
    _playLineVC.containerView = _playLineView;
    
    _measureVC.namidi = _namidi;
    _playLineVC.namidi = _namidi;
    
    _measureVC.scaleAssistant = _scaleAssistant;
    _playLineVC.scaleAssistant = _scaleAssistant;
    
    _measureView.documentView = _measureVC.view;
    _playLineView.documentView = _playLineVC.view;
    
    _measureVC.view.frame = _measureView.frame;
    _measureVC.view.needsLayout = YES;
    
    [_measureView observeScrollForScrollView:_conductorView x:YES y:NO];
    [_conductorView observeScrollForScrollView:_pianoRollView x:YES y:NO];
    [_pianoRollView observeScrollForScrollView:_velocityView x:YES y:NO];
    [_velocityView observeScrollForScrollView:_measureView x:YES y:NO];
    
    [_playLineView observeScrollForScrollView:_pianoRollView x:YES y:NO];
    
    [_keyboardView observeScrollForScrollView:_pianoRollView x:NO y:YES];
    [_pianoRollView observeScrollForScrollView:_keyboardView x:NO y:YES];
    
    [_horizontalSplitView setPosition:CGRectGetHeight(self.view.frame) - VELOCITY_VIEW_HEIGHT_MAX ofDividerAtIndex:0];
}

#pragma mark NSSplitViewDelegate

- (CGFloat)splitView:(NSSplitView *)splitView constrainMinCoordinate:(CGFloat)proposedMinimumPosition ofSubviewAt:(NSInteger)dividerIndex
{
    return CGRectGetHeight(self.view.frame) - VELOCITY_VIEW_HEIGHT_MAX;
}

- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedMaximumPosition ofSubviewAt:(NSInteger)dividerIndex
{
    return CGRectGetHeight(self.view.frame) - VELOCITY_VIEW_HEIGHT_MIN;
}

- (BOOL)splitView:(NSSplitView *)splitView shouldAdjustSizeOfSubview:(NSView *)view
{
    return splitView.subviews[0] == view;
}

@end
