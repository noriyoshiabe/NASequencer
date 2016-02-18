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
    return [NSString stringWithFormat:@"%.2f", 124.20];
}

- (NSString *)timeSign
{
    return [NSString stringWithFormat:@"%d/%d", 3, 4];
}

@end
