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

- (NSString *)identifier
{
    return @"Purchase";
}

- (NSImage *)toolbarItemImage
{
    return [NSImage imageNamed:@"purchase"];
}

- (NSString *)toolbarItemLabel
{
    return NSLocalizedString(@"Preference_Purchase", @"Purchase");
}

- (void)layout
{
    self.view.frame = self.view.bounds;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do view setup here.
}

- (IBAction)purchasePressed:(id)sender
{
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = @"Sorry, this function will be implemented on Version 0.5.0. 😀";
    [alert addButtonWithTitle:NSLocalizedString(@"OK", @"OK")];;
    [alert runModal];
}

- (IBAction)restorePurchasePressed:(id)sender
{
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = @"Sorry, this function will be implemented on Version 0.5.0. 😀";
    [alert addButtonWithTitle:NSLocalizedString(@"OK", @"OK")];;
    [alert runModal];
}

@end
