//
//  ConductorViewController.m
//  NAMIDI
//
//  Created by abechan on 2/17/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "ConductorViewController.h"

@interface ConductorViewController () <NAMidiRepresentationObserver, PlayerRepresentationObserver>

@end

@implementation ConductorViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    
    [self updateTempo];
    [self updateTimeSign];
    
    [_namidi addObserver:self];
    [_namidi.player addObserver:self];
}

- (void)viewDidDisappear
{
    [super viewDidDisappear];
    [_namidi removeObserver:self];
    [_namidi.player removeObserver:self];
}

- (NSString *)tempo
{
    return [NSString stringWithFormat:@"%.2f", _namidi.player.tempo];
}

- (NSString *)timeSign
{
    TimeSign timeSign = _namidi.player.timeSign;
    return [NSString stringWithFormat:@"%d/%d", timeSign.numerator, timeSign.denominator];
}

- (void)updateTempo
{
    [self willChangeValueForKey:@"tempo"];
    [self didChangeValueForKey:@"tempo"];
}

- (void)updateTimeSign
{
    [self willChangeValueForKey:@"timeSign"];
    [self didChangeValueForKey:@"timeSign"];
}

#pragma mark NAMidiRepresentationObserver

- (void)namidiDidParse:(NAMidiRepresentation *)namidi sequence:(SequenceRepresentation *)sequence parseInfo:(ParseInfoRepresentation *)parseInfo
{
    [self updateTempo];
    [self updateTimeSign];
}

#pragma mark PlayerRepresentationObserver

- (void)player:(PlayerRepresentation *)player onNotifyEvent:(PlayerEvent)event
{
    switch (event) {
        case PlayerEventTempoChange:
            [self updateTempo];
            break;
        case PlayerEventTimeSignChange:
            [self updateTimeSign];
            break;
        case PlayerEventRewind:
            [self updateTempo];
            [self updateTimeSign];
            break;
        default:
            break;
    }
}

@end
