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

#define EVENT_LIST_MIN_WIDTH 464.0
#define PIANO_ROLL_MIN_WIDTH 120.0

@interface DetailViewController () <NSSplitViewDelegate, SelectionViewControllerDelegate> {
    CGFloat _lastDividerPosition;
}

@property (weak) IBOutlet NSSplitView *verticalSplitView;
@property (weak) IBOutlet NSView *selectionView;
@property (weak) IBOutlet NSView *eventListView;
@property (weak) IBOutlet NSView *pianoRollView;
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
    
    _selectionVC.delegate = self;
    
    _selectionVC.namidi = _namidi;
    _eventListVC.namidi = _namidi;
    _pianoRollVC.namidi = _namidi;
    
    _pianoRollVC.scaleAssistant = _scaleAssistant;
    
    _selectionVC.trackSelection = _trackSelection;
    _eventListVC.trackSelection = _trackSelection;
    _pianoRollVC.trackSelection = _trackSelection;
    
    [_selectionView addSubviewWithFitConstraints:_selectionVC.view];
    [_pianoRollView addSubviewWithFitConstraints:_pianoRollVC.view];
    
    [_verticalSplitView setPosition:0 ofDividerAtIndex:0];
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    [self.view.window makeFirstResponder:self.view];
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    if (@selector(showEventList:) == menuItem.action) {
        return [_selectionVC validateMenuItem:menuItem];
    }
    else if (@selector(filterNoteEvent:) == menuItem.action
             || @selector(filterControlEvent:) == menuItem.action) {
        if (_selectionVC.listOpened) {
            return [_eventListVC validateMenuItem:menuItem];
        }
        else {
            return NO;
        }
    }
    
    return YES;
}

- (IBAction)showEventList:(id)sender
{
    [_selectionVC showEventList:sender];
}

- (IBAction)filterNoteEvent:(id)sender
{
    [_eventListVC filterNoteEvent:sender];
}

- (IBAction)filterControlEvent:(id)sender
{
    [_eventListVC filterControlEvent:sender];
}

- (CGPoint)scrollPoint
{
    return _pianoRollVC.scrollPoint;
}

- (void)setScrollPoint:(CGPoint)scrollPoint
{
    _pianoRollVC.scrollPoint = scrollPoint;
}

#pragma mark NSSplitViewDelegate

- (CGFloat)splitView:(NSSplitView *)splitView constrainMinCoordinate:(CGFloat)proposedMinimumPosition ofSubviewAt:(NSInteger)dividerIndex
{
    return _selectionVC.listOpened ? EVENT_LIST_MIN_WIDTH : 0.0;
}

- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedMaximumPosition ofSubviewAt:(NSInteger)dividerIndex
{
    return _selectionVC.listOpened ? CGRectGetWidth(_verticalSplitView.bounds) - PIANO_ROLL_MIN_WIDTH : 0.0;
}

- (BOOL)splitView:(NSSplitView *)splitView shouldAdjustSizeOfSubview:(NSView *)view
{
    return splitView.subviews[1] == view;
}

- (BOOL)splitView:(NSSplitView *)splitView shouldHideDividerAtIndex:(NSInteger)dividerIndex
{
    return _selectionVC.listOpened ? NO : YES;
}

- (void)splitViewDidResizeSubviews:(NSNotification *)notification
{
    if (_selectionVC.listOpened) {
        _lastDividerPosition = CGRectGetWidth(_verticalSplitView.subviews[0].frame);
    }
}

#pragma mark SelectionViewControllerDelegate

- (void)selectionViewControllerDidToggleListOpened:(SelectionViewController *)controller
{
    if (_selectionVC.listOpened) {
        CGFloat position = 0.0 < _lastDividerPosition ? _lastDividerPosition : EVENT_LIST_MIN_WIDTH;
        [_verticalSplitView setPosition:position ofDividerAtIndex:0];
        
        CGRect frame = _eventListVC.view.frame;
        frame.size.width = position;
        _eventListVC.view.frame = frame;
        [_eventListView addSubviewWithFitConstraints:_eventListVC.view];
    }
    else {
        [_eventListVC.view removeFromSuperview];
        [_verticalSplitView setPosition:0 ofDividerAtIndex:0];
    }
}

@end
