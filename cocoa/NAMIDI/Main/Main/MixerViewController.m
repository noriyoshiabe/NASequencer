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

@interface MixerViewController () <NAMidiRepresentationObserver, MixerRepresentationObserver, MixerChannelViewControllerDelegate> {
    MixerRepresentation *_mixer;
}

@property (strong) IBOutlet NSView *masterChannelView;
@property (weak) IBOutlet LevelIndicator *indicatorL;
@property (weak) IBOutlet LevelIndicator *indicatorR;
@property (strong, nonatomic) NSMutableArray *controllers;
@property (readonly, nonatomic) int L;
@property (readonly, nonatomic) int R;
@end

@implementation MixerViewController

- (instancetype)init
{
    self = [super init];
    if (self) {
        _controllers = [NSMutableArray array];
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _mixer = _namidi.mixer;
    
    for (int i = 0; i < 16; ++i) {
        MixerChannelViewController *channelVC = [[MixerChannelViewController alloc] init];
        channelVC.delegate = self;
        channelVC.mixer = _mixer;
        channelVC.channel = i + 1;
        [_controllers addObject:channelVC];
        [self.view addSubview:channelVC.view];
    }
    
    [self.view addSubview:_masterChannelView];
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    
    [_indicatorL bind:@"intValue" toObject:self withKeyPath:@"L" options:nil];
    [_indicatorR bind:@"intValue" toObject:self withKeyPath:@"R" options:nil];
    
    [_namidi addObserver:self];
    [_namidi.mixer addObserver:self];
    
    [self layout];
}

- (void)viewDidDisappear
{
    [super viewDidDisappear];
    
    [_indicatorL unbind:@"intValue"];
    [_indicatorR unbind:@"intValue"];
    
    [_namidi removeObserver:self];
    [_namidi.mixer removeObserver:self];
}

- (void)layout
{
    CGFloat height = 0;
    CGFloat width = 0;
    
    for (int i = 0; i < 16; ++i) {
        if (_namidi.sequence.channels[i].exist) {
            MixerChannelViewController *channelVC = _controllers[i];
            height += channelVC.view.frame.size.height;
            width = MAX(width, channelVC.view.frame.size.width);
        }
    }
    
    height += _masterChannelView.frame.size.height;
    
    self.view.frame = CGRectMake(0, self.view.frame.origin.y, width, height);
    
    CGFloat y = 0;
    for (int i = 0; i < 16; ++i) {
        MixerChannelViewController *channelVC = _controllers[i];
        
        if (_namidi.sequence.channels[i].exist) {
            channelVC.view.hidden = NO;
            channelVC.view.frame = CGRectMake(0, y, width, channelVC.view.frame.size.height);
            y += channelVC.view.frame.size.height;
        }
        else {
            channelVC.view.hidden = YES;
        }
    }
    
    _masterChannelView.frame = CGRectMake(0, y, width, _masterChannelView.frame.size.height);
}

- (int)L
{
    return _mixer.level.L;
}

- (int)R
{
    return _mixer.level.R;
}

#pragma mark NAMidiRepresentationObserver

- (void)namidiDidParse:(NAMidiRepresentation *)namidi sequence:(SequenceRepresentation *)sequence parseInfo:(ParseInfoRepresentation *)parseInfo
{
    [self layout];
}

#pragma mark MixerRepresentationObserver

- (void)mixerOnLevelUpdate:(MixerRepresentation *)mixer
{
    [self notifyValueChangeForKey:@"L"];
    [self notifyValueChangeForKey:@"R"];
}

#pragma mark MixerChannelViewControllerDelegate

- (void)mixerChannelViewController:(MixerChannelViewController *)controller didSelectChannel:(MixerChannelRepresentation *)mixerChannel
{
    [_delegate mixerViewController:self didSelectChannel:mixerChannel];
}

@end
