//
//  PurchaseViewController.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PurchaseViewController.h"
#import "IAP.h"
#import "ColorButton.h"

@interface PurchaseViewController () <IAPObserver>
@property (weak) IBOutlet NSView *additionalViewContainer;
@property (weak) IBOutlet NSView *purchaseView;
@property (weak) IBOutlet NSView *thanksView;
@property (weak) IBOutlet NSTextField *priceLabel;
@property (weak) IBOutlet NSButton *purchaseButton;
@property (weak) IBOutlet NSButton *restorePurchaseButton;
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
    
    [[IAP sharedInstance] findIAPProduct:kIAPProductFullVersion found:^(NSString *productID, int quantity) {
        [self showThanksView];
    } notFound:^(NSString *productID) {
        [self showPurchaseView];
    }];
    
    [[IAP sharedInstance] addObserver:self];
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];
    [[IAP sharedInstance] removeObserver:self];
}

- (void)showPurchaseView
{
    [_purchaseView removeFromSuperview];
    [_thanksView removeFromSuperview];
    
    [_additionalViewContainer addSubview:_purchaseView];
    
    _purchaseButton.enabled = NO;
    _restorePurchaseButton.enabled = NO;
    
    if (![IAP sharedInstance].canMakePayments) {
        _priceLabel.stringValue = NSLocalizedString(@"Purchase_CannotMakePayments", @"Sorry, you are not allowed to make payments.");
    }
    else {
        _priceLabel.stringValue = NSLocalizedString(@"Purchase_PriceLoading", @"Loading…");
        
        [[IAP sharedInstance] requestProductInfo:@[kIAPProductFullVersion] callback:^(NSArray *productInfos) {
            for (IAPProductInfo *product in productInfos) {
                if ([product.productId isEqualToString:kIAPProductFullVersion]) {
                    NSString *format = NSLocalizedString(@"Purchase_PriceFormat", @"%@, for all your Macs");
                    _priceLabel.stringValue = [NSString stringWithFormat:format, product.displayPrice];
                }
            }
            
            _purchaseButton.enabled = YES;
            _restorePurchaseButton.enabled = YES;
        }];
    }
}

- (void)showThanksView
{
    [_purchaseView removeFromSuperview];
    [_thanksView removeFromSuperview];
    
    [_additionalViewContainer addSubview:_thanksView];
}

- (IBAction)purchasePressed:(id)sender
{
    [[IAP sharedInstance] purchase:kIAPProductFullVersion];
}

- (IBAction)restorePurchasePressed:(id)sender
{
    [[IAP sharedInstance] restorePurchase:kIAPProductFullVersion];
}

- (IBAction)tweetPressed:(id)sender
{
    NSString *text = NSLocalizedString(@"Preference_PurchaseTweetMessage", @"NASequencer, the text-based MIDI composer. For Composition, Transcription, Karaoke, etc.");
    NSString *encodedText = [text stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet alphanumericCharacterSet]];
    NSString *encodedUrl = [@"https://nasequencer.com" stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet alphanumericCharacterSet]];
    NSURL *url = [NSURL URLWithString:[NSString stringWithFormat:@"https://x.com/intent/post?text=%@&url=%@", encodedText, encodedUrl]];
    [[NSWorkspace sharedWorkspace] openURL: url];
}

- (IBAction)shareOnFacebookPressed:(id)sender
{
    NSString *encodedUrl = [@"https://nasequencer.com" stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet alphanumericCharacterSet]];
    NSURL *url = [NSURL URLWithString:[NSString stringWithFormat:@"https://www.facebook.com/sharer/sharer.php?u=%@", encodedUrl]];
    [[NSWorkspace sharedWorkspace] openURL: url];
}

#pragma mark IAPObserver

- (void)iap:(id<IAPDelegate>)iap didUpdateTransaction:(IAPTransaction *)transaction
{
    switch (transaction.transactionState) {
        case IAPTransactionStateUnknown:
        case IAPTransactionStateCanceled:
            break;
        case IAPTransactionStatePurchasing:
            _purchaseButton.enabled = NO;
            _restorePurchaseButton.enabled = NO;
            break;
        case IAPTransactionStateDeferred:
            break;
        case IAPTransactionStateFailed:
            _purchaseButton.enabled = YES;
            _restorePurchaseButton.enabled = YES;
            break;
        case IAPTransactionStatePurchased:
        case IAPTransactionStateRestored:
            [[IAP sharedInstance] findIAPProduct:kIAPProductFullVersion found:^(NSString *productID, int quantity) {
                [self showThanksView];
            } notFound:^(NSString *productID) {
                [self showPurchaseView];
            }];
            break;
    }
}

@end


#pragma mark For Social Button

@interface PurchaseViewShareButton : ColorButton
@end

@implementation PurchaseViewShareButton

- (void)awakeFromNib
{
    [super awakeFromNib];
    [self addTrackingRect:self.bounds owner:self userData:nil assumeInside:NO];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    if (!self.enabled) {
        return;
    }
    
    self.state = NSControlStateValueOn;
}

- (void)mouseUp:(NSEvent *)theEvent
{
    if (!self.enabled) {
        return;
    }
    
    self.state = NSControlStateValueOff;
    
    [self sendAction:self.action to:self.target];
}

- (void)mouseExited:(NSEvent *)theEvent
{
    if (!self.enabled) {
        return;
    }
    
    self.state = NSControlStateValueOff;
}

@end
