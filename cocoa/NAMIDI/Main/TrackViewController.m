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
        [_controllers addObject:channelVC];
        [self.view addSubview:channelVC.view];
    }
    
    [self.view addSubview:_masterTrackView];
    
    [self layout];
}

- (void)layout
{
    self.view.frame = CGRectMake(0, 0, self.view.bounds.size.width, 0);
    
    CGFloat y = 0;
    CGFloat width = 0;
    for (int i = 0; i < 16; ++i) {
        TrackChannelViewController *channelVC = _controllers[i];
        y += channelVC.view.frame.size.height;
        width = MAX(width, channelVC.view.frame.size.width);
        
        channelVC.view.frame = CGRectFromRectWithXY(channelVC.view.frame, 0, -y);
        [self.view addSubview:channelVC.view];
    }
    
    y += _masterTrackView.frame.size.height;
    _masterTrackView.frame = CGRectMake(0, -y, width, _masterTrackView.frame.size.height);
    self.view.frame = CGRectMake(0, 0, width, y);
}

@end
