//
//  MixerChannelViewController.m
//  NAMIDI
//
//  Created by abechan on 2/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "MixerChannelViewController.h"
#import "Color.h"

@interface MixerChannelViewController ()
@property (weak) IBOutlet NSView *underLine;
@end

@implementation MixerChannelViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    _underLine.wantsLayer = YES;
    _underLine.layer.backgroundColor = [Color gray].CGColor;
}

//- (NSArray *)synthsizerIdentifiers
//{
//    return @[@"TEST1", @"TEST2"];
//}

@end
