//
//  PurchaseViewController.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PurchaseViewController.h"

@interface PurchaseViewController ()

@end

@implementation PurchaseViewController

- (NSString *)nibName
{
    return @"PurchaseViewController";
}

- (NSString *)identifier
{
    return @"Purchase";
}

- (NSImage *)toolbarItemImage
{
    return [NSImage imageNamed:NSImageNameApplicationIcon];
}

- (NSString *)toolbarItemLabel
{
    return NSLocalizedString(@"Preference_Purchase", @"Purchase");
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do view setup here.
}

@end
