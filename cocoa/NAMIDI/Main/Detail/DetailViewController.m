//
//  DetailViewController.m
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "DetailViewController.h"
#import "SelectionViewController.h"
#import "EventListViewController.h"
#import "PianoRollViewController.h"

#define VERTICAL_SPLIT_LEFT_MAX 400
#define VERTICAL_SPLIT_LEFT_MIN 232

#define SELECTION_VIEW_HEIGHT_NARROW 25
#define SELECTION_VIEW_HEIGHT_WIDE 46

@interface DetailViewController () <NSSplitViewDelegate>
@property (weak) IBOutlet NSSplitView *verticalSplitView;
@property (weak) IBOutlet NSView *selectionView;
@property (weak) IBOutlet NSView *eventListView;
@property (weak) IBOutlet NSView *pianoRollView;
@property (weak) IBOutlet NSLayoutConstraint *selectionViewHeightConstraint;
@property (strong, nonatomic) SelectionViewController *selectionVC;
@property (strong, nonatomic) EventListViewController *eventListVC;
@property (strong, nonatomic) PianoRollViewController *pianoRollVC;
@end

@implementation DetailViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _verticalSplitView.delegate = self;
    
    _selectionVC = [[SelectionViewController alloc] init];
    _eventListVC = [[EventListViewController alloc] init];
    _pianoRollVC = [[PianoRollViewController alloc] init];
    
    _eventListVC.namidi = _namidi;
    _pianoRollVC.namidi = _namidi;
    
    _pianoRollVC.scaleAssistant = _scaleAssistant;
    
    _selectionVC.trackSelection = _trackSelection;
    _eventListVC.trackSelection = _trackSelection;
    _pianoRollVC.trackSelection = _trackSelection;
    
    [_selectionView addSubviewWithFitConstraints:_selectionVC.view];
    [_eventListView addSubviewWithFitConstraints:_eventListVC.view];
    [_pianoRollView addSubviewWithFitConstraints:_pianoRollVC.view];
    
    [_verticalSplitView setPosition:VERTICAL_SPLIT_LEFT_MAX ofDividerAtIndex:0];
}

#pragma mark NSSplitViewDelegate

- (CGFloat)splitView:(NSSplitView *)splitView constrainMinCoordinate:(CGFloat)proposedMinimumPosition ofSubviewAt:(NSInteger)dividerIndex
{
    return VERTICAL_SPLIT_LEFT_MIN;
}

- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedMaximumPosition ofSubviewAt:(NSInteger)dividerIndex
{
    return VERTICAL_SPLIT_LEFT_MAX;
}

- (BOOL)splitView:(NSSplitView *)splitView shouldAdjustSizeOfSubview:(NSView *)view
{
    return splitView.subviews[1] == view;
}

- (void)splitViewDidResizeSubviews:(NSNotification *)notification
{
    if (VERTICAL_SPLIT_LEFT_MAX > CGRectGetWidth(_verticalSplitView.subviews[0].frame)) {
        _selectionViewHeightConstraint.constant = SELECTION_VIEW_HEIGHT_WIDE;
    }
    else {
        _selectionViewHeightConstraint.constant = SELECTION_VIEW_HEIGHT_NARROW;
    }
}

@end
