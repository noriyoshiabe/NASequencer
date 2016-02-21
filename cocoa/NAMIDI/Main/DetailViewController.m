//
//  DetailViewController.m
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "DetailViewController.h"

#define VERTICAL_SPLIT_LEFT_MAX 400
#define VERTICAL_SPLIT_LEFT_MIN 240

#define HORIZONTAL_SPLIT_BOTTOM_MAX 160
#define HORIZONTAL_SPLIT_BOTTOM_MIN 40

#define SELECTION_VIEW_HEIGHT_NARROW 25
#define SELECTION_VIEW_HEIGHT_WIDE 46

@interface DetailViewController () <NSSplitViewDelegate>
@property (weak) IBOutlet NSSplitView *verticalSplitView;
@property (weak) IBOutlet NSSplitView *horizontalSplitView;
@property (weak) IBOutlet NSView *selectionView;
@property (weak) IBOutlet NSView *eventListView;
@property (weak) IBOutlet NSView *measureView;
@property (weak) IBOutlet NSView *conductorView;
@property (weak) IBOutlet NSView *pianoRollView;
@property (weak) IBOutlet NSView *velocityView;
@property (weak) IBOutlet NSLayoutConstraint *selectionViewHeightConstraint;
@end

@implementation DetailViewController

- (void)awakeFromNib
{
    _selectionView.wantsLayer = YES;
    _eventListView.wantsLayer = YES;
    _measureView.wantsLayer = YES;
    _conductorView.wantsLayer = YES;
    _pianoRollView.wantsLayer = YES;
    _velocityView.wantsLayer = YES;
    
    _selectionView.layer.backgroundColor = [NSColor redColor].CGColor;
    _eventListView.layer.backgroundColor = [NSColor blueColor].CGColor;
    _measureView.layer.backgroundColor = [NSColor greenColor].CGColor;
    _conductorView.layer.backgroundColor = [NSColor yellowColor].CGColor;
    _pianoRollView.layer.backgroundColor = [NSColor magentaColor].CGColor;
    _velocityView.layer.backgroundColor = [NSColor cyanColor].CGColor;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _verticalSplitView.delegate = self;
    _horizontalSplitView.delegate = self;
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    
    [_verticalSplitView setPosition:VERTICAL_SPLIT_LEFT_MAX ofDividerAtIndex:0];
    [_horizontalSplitView setPosition:CGRectGetHeight(self.view.frame) - HORIZONTAL_SPLIT_BOTTOM_MAX ofDividerAtIndex:0];
}

#pragma mark NSSplitViewDelegate

- (CGFloat)splitView:(NSSplitView *)splitView constrainMinCoordinate:(CGFloat)proposedMinimumPosition ofSubviewAt:(NSInteger)dividerIndex
{
    if (splitView.vertical) {
        return VERTICAL_SPLIT_LEFT_MIN;
    }
    else {
        return CGRectGetHeight(self.view.frame) - HORIZONTAL_SPLIT_BOTTOM_MAX;
    }
}

- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedMaximumPosition ofSubviewAt:(NSInteger)dividerIndex
{
    if (splitView.vertical) {
        return VERTICAL_SPLIT_LEFT_MAX;
    }
    else {
        return CGRectGetHeight(self.view.frame) - HORIZONTAL_SPLIT_BOTTOM_MIN;
    }
}

- (BOOL)splitView:(NSSplitView *)splitView shouldAdjustSizeOfSubview:(NSView *)view
{
    if (splitView.vertical) {
        return splitView.subviews[1] == view;
    }
    else {
        return splitView.subviews[0] == view;
    }
}

- (void)splitViewDidResizeSubviews:(NSNotification *)notification
{
    if (notification.object == _verticalSplitView) {
        if (VERTICAL_SPLIT_LEFT_MAX > CGRectGetWidth(_verticalSplitView.subviews[0].frame)) {
            _selectionViewHeightConstraint.constant = SELECTION_VIEW_HEIGHT_WIDE;
        }
        else {
            _selectionViewHeightConstraint.constant = SELECTION_VIEW_HEIGHT_NARROW;
        }
    }
}

@end
