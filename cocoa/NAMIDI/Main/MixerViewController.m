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
@property (assign, nonatomic) int L;
@property (assign, nonatomic) int R;
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
    CGFloat height = 0;
    CGFloat width = 0;
    
    for (int i = 0; i < 16; ++i) {
        MixerChannelViewController *channelVC = _controllers[i];
        height += channelVC.view.frame.size.height;
        width = MAX(width, channelVC.view.frame.size.width);
    }
    
    height += _masterChannelView.frame.size.height;
    
    self.view.frame = CGRectMake(0, self.view.frame.origin.y, width, height);
    
    CGFloat y = 0;
    for (int i = 0; i < 16; ++i) {
        MixerChannelViewController *channelVC = _controllers[i];
        channelVC.view.frame = CGRectMake(0, y, width, channelVC.view.frame.size.height);
        y += channelVC.view.frame.size.height;
    }
    
    _masterChannelView.frame = CGRectMake(0, y, width, _masterChannelView.frame.size.height);
}

@end
