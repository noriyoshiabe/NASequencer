//
//  TrackViewController.m
//  NAMIDI
//
//  Created by abechan on 2/17/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "TrackViewController.h"
#import "TrackChannelViewController.h"

@interface TrackViewController ()
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
        channelVC.scaleAssistant = _scaleAssistant;
        [_controllers addObject:channelVC];
        [self.view addSubview:channelVC.view];
    }
    
    [self.view addSubview:_masterTrackView];
    
    [_scaleAssistant addObserver:self forKeyPath:@"scale" options:0 context:NULL];
    
    [self layout];
}

- (void)dealloc
{
    [_scaleAssistant removeObserver:self forKeyPath:@"scale"];
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
    CGFloat height = 0;
    CGFloat width = 0;
    
    for (int i = 0; i < 16; ++i) {
        TrackChannelViewController *channelVC = _controllers[i];
        height += channelVC.view.frame.size.height;
        width = MAX(width, channelVC.view.frame.size.width);
    }
    
    height += _masterTrackView.frame.size.height;
    
    self.view.frame = CGRectMake(0, self.view.frame.origin.y, width, height);
    
    CGFloat y = 0;
    for (int i = 0; i < 16; ++i) {
        TrackChannelViewController *channelVC = _controllers[i];
        channelVC.view.frame = CGRectMake(0, y, width, channelVC.view.frame.size.height);
        y += channelVC.view.frame.size.height;
    }
    
    _masterTrackView.frame = CGRectMake(0, y, width, _masterTrackView.frame.size.height);
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    if (![_scaleAssistant scrollWheel:theEvent]) {
        [super scrollWheel:theEvent];
    }
}

@end
