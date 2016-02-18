//
//  MixerViewController.m
//  NAMIDI
//
//  Created by abechan on 2/17/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MixerViewController.h"
#import "MixerChannelViewController.h"
#import "LevelIndicator.h"

@interface MixerViewController ()
@property (strong) IBOutlet NSView *masterChannelView;
@property (weak) IBOutlet LevelIndicator *indicatorL;
@property (weak) IBOutlet LevelIndicator *indicatorR;
@property (strong, nonatomic) NSMutableArray *controllers;
@end

@implementation MixerViewController

- (instancetype)init
{
    self = [super init];
    if (self) {
        self.controllers = [NSMutableArray array];
        
        _L = -200;
        _R = -200;
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    [_indicatorL bind:@"intValue" toObject:self withKeyPath:@"L" options:nil];
    [_indicatorR bind:@"intValue" toObject:self withKeyPath:@"R" options:nil];
    
    for (int i = 0; i < 16; ++i) {
        MixerChannelViewController *channelVC = [[MixerChannelViewController alloc] init];
        channelVC.channel = i + 1;
        [_controllers addObject:channelVC];
        [self.view addSubview:channelVC.view];
    }
    
    [self.view addSubview:_masterChannelView];
    
    [self layout];
}

- (void)layout
{
    self.view.frame = CGRectMake(0, 0, self.view.bounds.size.width, 0);
    
    CGFloat y = 0;
    for (int i = 0; i < 16; ++i) {
        MixerChannelViewController *channelVC = _controllers[i];
        y += channelVC.view.frame.size.height;
        channelVC.view.frame = CGRectFromRectWithXY(channelVC.view.frame, 0, -y);
        [self.view addSubview:channelVC.view];
    }
    
    y += _masterChannelView.frame.size.height;
    _masterChannelView.frame = CGRectFromRectWithXY(_masterChannelView.frame, 0, -y);
    self.view.frame = CGRectMake(0, 0, self.view.frame.size.width, y);
}

@end
