//
//  MainViewController.m
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MainViewController.h"
#import "ConductorViewController.h"
#import "ConductorTrackViewController.h"
#import "MeasureViewController.h"
#import "MixerViewController.h"
#import "TrackViewController.h"
#import "PlayLineViewController.h"
#import "TipsViewController.h"
#import "MeasureScaleAssistant.h"
#import "SynchronizedScrollView.h"
#import "Color.h"

@interface MainViewController () <NAMidiRepresentationObserver, SynchronizedScrollViewDelegate, MixerViewControllerDelegate>
@property (weak) IBOutlet NSView *conductorView;
@property (weak) IBOutlet SynchronizedScrollView *mixerView;
@property (weak) IBOutlet SynchronizedScrollView *measureView;
@property (weak) IBOutlet SynchronizedScrollView *conductorTrackView;
@property (weak) IBOutlet SynchronizedScrollView *trackView;
@property (weak) IBOutlet SynchronizedScrollView *playLineView;
@property (weak) IBOutlet NSView *tipsView;
@property (strong, nonatomic) ConductorViewController *conductorVC;
@property (strong, nonatomic) MeasureViewController *measureVC;
@property (strong, nonatomic) ConductorTrackViewController *conductorTrackVC;
@property (strong, nonatomic) MixerViewController *mixerVC;
@property (strong, nonatomic) TrackViewController *trackVC;
@property (strong, nonatomic) PlayLineViewController *playLineVC;
@property (strong, nonatomic) TipsViewController *tipsVC;
@property (weak) IBOutlet NSView *horizontalLine;
@property (weak) IBOutlet NSView *verticalLine;
@end

@implementation MainViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _conductorView.layer.backgroundColor = [Color darkGray].CGColor;
    _mixerView.backgroundColor = [Color darkGray];
    _measureView.backgroundColor = [Color darkGray];
    _conductorTrackView.backgroundColor = [Color darkGray];
    
    _horizontalLine.layer.backgroundColor = [Color gray].CGColor;
    _verticalLine.layer.backgroundColor = [Color gray].CGColor;
    
    _conductorVC = [[ConductorViewController alloc] init];
    _measureVC = [[MeasureViewController alloc] init];
    _conductorTrackVC = [[ConductorTrackViewController alloc] init];
    _mixerVC = [[MixerViewController alloc] init];
    _trackVC = [[TrackViewController alloc] init];
    _playLineVC = [[PlayLineViewController alloc] init];
    _playLineVC.containerView = _playLineView;
    _tipsVC = [[TipsViewController alloc] init];
    
    _mixerVC.delegate = self;
    
    _conductorVC.namidi = _namidi;
    _measureVC.namidi = _namidi;
    _conductorTrackVC.namidi = _namidi;
    _mixerVC.namidi = _namidi;
    _trackVC.namidi = _namidi;
    _playLineVC.namidi = _namidi;
    _tipsVC.namidi =_namidi;
    
    _measureVC.scaleAssistant = _scaleAssistant;
    _conductorTrackVC.scaleAssistant = _scaleAssistant;
    _trackVC.scaleAssistant = _scaleAssistant;
    _playLineVC.scaleAssistant = _scaleAssistant;
    
    _conductorTrackVC.trackSelection = _trackSelection;
    _trackVC.trackSelection = _trackSelection;
    
    _playLineVC.mainWindowContext = _mainWindowContext;
    
    _conductorVC.view.frame = _conductorView.bounds;
    [_conductorView addSubview:_conductorVC.view];
    
    _measureView.documentView = _measureVC.view;
    _conductorTrackView.documentView = _conductorTrackVC.view;
    _mixerView.documentView = _mixerVC.view;
    _trackView.documentView = _trackVC.view;
    _playLineView.documentView = _playLineVC.view;
    
    [_tipsView addSubviewWithFitConstraints:_tipsVC.view];
    
    _trackView.delegate = self;
    _playLineView.userInteractionEnabled = NO;
    
    [_measureView observeScrollForScrollView:_conductorTrackView x:YES y:NO];
    [_conductorTrackView observeScrollForScrollView:_trackView x:YES y:NO];
    [_trackView observeScrollForScrollView:_playLineView x:YES y:NO];
    [_playLineView observeScrollForScrollView:_measureView x:YES y:NO];
    
    [_mixerView observeScrollForScrollView:_trackView x:NO y:YES];
    [_trackView observeScrollForScrollView:_mixerView x:NO y:YES];
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    [self updateTipsVisibility];
    [_namidi addObserver:self];
}

- (void)viewDidAppear
{
    [self.view.window makeMainWindow];
    [self.view.window makeFirstResponder:self.view];
}

- (void)viewDidDisappear
{
    [super viewDidDisappear];
    [_namidi removeObserver:self];
}

- (void)updateTipsVisibility
{
    _tipsView.hidden = _namidi.sequence.eventsOfConductorTrack.count < _namidi.sequence.events.count;
}

- (CGPoint)scrollPoint
{
    return _measureView.contentView.bounds.origin;
}

- (void)setScrollPoint:(CGPoint)scrollPoint
{
    [_measureView.contentView scrollToPoint:scrollPoint];
}

#pragma mark Keyboad event

- (void)keyDown:(NSEvent *)theEvent
{
    unichar key = [[theEvent charactersIgnoringModifiers] characterAtIndex:0];
    if (NSCarriageReturnCharacter == key && _trackSelection.isAnyTrackSelected) {
        [_delegate mainViewControllerDidEnterSelection:self];
    }
    else {
        [super keyDown:theEvent];
    }
}

#pragma mark Menu Action

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    if (@selector(deselectAll:) == menuItem.action) {
        return _trackSelection.isAnyTrackSelected;
    }
    else {
        return YES;
    }
}

- (IBAction)selectAll:(id)sender
{
    [_trackSelection selectAll];
}

- (IBAction)deselectAll:(id)sender
{
    [_trackSelection deselectAll];
}

- (void)cancelOperation:(id)sender
{
    [_trackSelection deselectAll];
}

#pragma mark NAMidiRepresentationObserver

- (void)namidiDidParse:(NAMidiRepresentation *)namidi sequence:(SequenceRepresentation *)sequence parseInfo:(ParseInfoRepresentation *)parseInfo
{
    [self updateTipsVisibility];
}

#pragma mark SynchronizedScrollViewDelegate

- (BOOL)synchronizedScrollView:(SynchronizedScrollView *)scrollView shouldMouseDown:(NSEvent *)theEvent
{
    [_trackSelection deselectAll];
    return YES;
}

#pragma mark MixerViewControllerDelegate

- (void)mixerViewController:(MixerViewController *)controller didSelectPresetButtonWithChannel:(MixerChannelRepresentation *)mixerChannel
{
    [_delegate mainViewController:self didSelectPresetButtonWithChannel:mixerChannel];
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
