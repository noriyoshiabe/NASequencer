//
//  PurchaseViewController.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PurchaseViewController.h"
#import "IAP.h"

@interface PurchaseViewController ()

@property (weak) IBOutlet NSTextField *priceLabel;
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

- (void)viewWillAppear
{
    [super viewWillAppear];
    
    _priceLabel.stringValue = NSLocalizedString(@"Preference_PurchasePriceLoading", @"Loading…");
    
    [[IAP sharedInstance] requestProductInfo:@[kIAPProductFullVersion] callback:^(SKProductsResponse *response) {
        
        NSNumberFormatter *formatter = [[NSNumberFormatter alloc] init];
        // TODO replace to product's locale
        formatter.locale = [NSLocale localeWithLocaleIdentifier:@"en_US"];
        formatter.numberStyle = NSNumberFormatterCurrencyISOCodeStyle;
        formatter.formatterBehavior = NSNumberFormatterBehavior10_4;
        
        NSString *format = NSLocalizedString(@"Preference_PurchasePriceFormat", @"%@, for all your Macs");
        // TODO replace to product's price;
        _priceLabel.stringValue = [NSString stringWithFormat:format, [formatter stringFromNumber:[NSDecimalNumber numberWithFloat:24.99]]];
    }];
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
