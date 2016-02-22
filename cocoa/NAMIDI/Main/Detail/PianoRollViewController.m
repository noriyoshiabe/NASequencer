//
//  PianoRollViewController.m
//  NAMIDI
//
//  Created by abechan on 2/22/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PianoRollViewController.h"

#define VELOCITY_VIEW_HEIGHT_MAX 160
#define VELOCITY_VIEW_HEIGHT_MIN 40

@interface PianoRollViewController () <NSSplitViewDelegate>
@property (weak) IBOutlet NSSplitView *horizontalSplitView;
@property (weak) IBOutlet NSView *measureView;
@property (weak) IBOutlet NSView *conductorView;
@property (weak) IBOutlet NSView *pianoRollView;
@property (weak) IBOutlet NSView *velocityView;
@end

@implementation PianoRollViewController

- (void)awakeFromNib
{
    _measureView.wantsLayer = YES;
    _conductorView.wantsLayer = YES;
    _pianoRollView.wantsLayer = YES;
    _velocityView.wantsLayer = YES;
    
    _measureView.layer.backgroundColor = [NSColor greenColor].CGColor;
    _conductorView.layer.backgroundColor = [NSColor yellowColor].CGColor;
    _pianoRollView.layer.backgroundColor = [NSColor magentaColor].CGColor;
    _velocityView.layer.backgroundColor = [NSColor cyanColor].CGColor;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _horizontalSplitView.delegate = self;
    
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
