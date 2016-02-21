//
//  ConductorViewController.m
//  NAMIDI
//
//  Created by abechan on 2/17/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "ConductorViewController.h"

@interface ConductorViewController ()

@end

@implementation ConductorViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
}

- (NSString *)tempo
{
    // TODO notify change
    return [NSString stringWithFormat:@"%.2f", _namidi.player.tempo];
}

- (NSString *)timeSign
{
    // TODO notify change
    TimeSign timeSign = _namidi.player.timeSign;
    return [NSString stringWithFormat:@"%d/%d", timeSign.numerator, timeSign.denominator];
}

@end
