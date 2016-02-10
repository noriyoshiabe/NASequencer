//
//  SynthesizerViewController.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "SynthesizerViewController.h"

@interface SynthesizerViewController ()

@end

@implementation SynthesizerViewController

- (instancetype)init
{
    return [super initWithNibName:@"SynthesizerViewController" bundle:nil];
}

- (NSString *)identifier
{
    return @"Synthesizer";
}

- (NSImage *)toolbarItemImage
{
    return [NSImage imageNamed:NSImageNameApplicationIcon];
}

- (NSString *)toolbarItemLabel
{
    return NSLocalizedString(@"Preference_Synthesizer", @"Synthesizer");
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do view setup here.
}

@end
