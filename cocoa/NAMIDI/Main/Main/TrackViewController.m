//
//  TrackViewController.m
//  NAMIDI
//
//  Created by abechan on 2/17/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "TrackViewController.h"
#import "TrackChannelViewController.h"

@interface TrackViewController () <NAMidiRepresentationObserver>
@property (strong) IBOutlet NSView *masterTrackView;
@property (strong, nonatomic) NSMutableArray *controllers;
@end

@implementation TrackViewController

- (instancetype)init
{
    self = [super init];
    if (self) {
        self.controllers = [NSMutableArray array];
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    for (int i = 0; i < 16; ++i) {
        TrackChannelViewController *channelVC = [[TrackChannelViewController alloc] init];
        channelVC.channel = i + 1;
        channelVC.namidi = _namidi;
        channelVC.scaleAssistant = _scaleAssistant;
        channelVC.trackSelection = _trackSelection;
        [_controllers addObject:channelVC];
        [self.view addSubview:channelVC.view];
    }
    
    [self.view addSubview:_masterTrackView];
}

- (void)viewDidAppear
{
    [super viewDidAppear];
    
    [_scaleAssistant addObserver:self forKeyPath:@"scale" options:0 context:NULL];
    [_namidi addObserver:self];
    
    [self layout];
}

- (void)viewDidDisappear
{
    [super viewDidDisappear];
    
    [_scaleAssistant removeObserver:self forKeyPath:@"scale"];
    [_namidi removeObserver:self];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSString *,id> *)change context:(void *)context
{
    if (object == _scaleAssistant) {
        CGFloat width = 0;
        for (int i = 0; i < 16; ++i) {
            TrackChannelViewController *channelVC = _controllers[i];
            [channelVC.view layout];
            width = MAX(width, channelVC.view.frame.size.width);
        }
        
        _masterTrackView.frame = CGRectMake(0, _masterTrackView.frame.origin.y, width, _masterTrackView.frame.size.height);
        self.view.frame = CGRectMake(0, self.view.frame.origin.y, width, self.view.frame.size.height);
    }
}

- (void)layout
{
    CGFloat width = _scaleAssistant.viewWidth;
    CGFloat height = 0;
    
    for (int i = 0; i < 16; ++i) {
        if (_namidi.sequence.channels[i].exist) {
            TrackChannelViewController *channelVC = _controllers[i];
            height += channelVC.view.frame.size.height;
        }
    }
    
    height += _masterTrackView.frame.size.height;
    
    self.view.frame = CGRectMake(0, self.view.frame.origin.y, width, height);
    
    CGFloat y = 0;
    for (int i = 0; i < 16; ++i) {
        TrackChannelViewController *channelVC = _controllers[i];
        
        if (_namidi.sequence.channels[i].exist) {
            channelVC.view.hidden = NO;
            channelVC.view.frame = CGRectMake(0, y, width, channelVC.view.frame.size.height);
            y += channelVC.view.frame.size.height;
        }
        else {
            channelVC.view.hidden = YES;
        }
    }
    
    _masterTrackView.frame = CGRectMake(0, y, width, _masterTrackView.frame.size.height);
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    if (![_scaleAssistant scrollWheel:theEvent]) {
        [super scrollWheel:theEvent];
    }
}

- (void)mouseDown:(NSEvent *)theEvent
{
    CGPoint point = [self.view convertPoint:theEvent.locationInWindow fromView:self.view.window.contentView];
    
    for (int i = 0; i < 16; ++i) {
        TrackChannelViewController *channelVC = _controllers[i];
        if (CGRectContainsPoint(channelVC.view.frame, point)) {
            [_trackSelection click:channelVC.channel event:theEvent];
            return;
        }
    }
    
    [_trackSelection click:-1 event:theEvent];
}

#pragma mark NAMidiRepresentationObserver

- (void)namidiDidParse:(NAMidiRepresentation *)namidi sequence:(SequenceRepresentation *)sequence parseInfo:(ParseInfoRepresentation *)parseInfo
{
    [self layout];
}

@end
