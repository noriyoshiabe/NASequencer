//
//  PurchaseViewController.m
//  NAMIDI
//
//  Created by abechan on 2/10/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "PurchaseViewController.h"
#import "IAP.h"
#import "ReceiptVerifier.h"
#import "ColorButton.h"

@interface PurchaseViewController () <ReceiptVerifierDelegate, IAPObserver>
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
    [self verifyReceipt];
    [[IAP sharedInstance] addObserver:self];
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];
    [[IAP sharedInstance] removeObserver:self];
}

- (void)verifyReceipt
{
    NSURL *receiptURL = [[NSBundle mainBundle] appStoreReceiptURL];
    NSURL *appleCert = [[NSBundle mainBundle] URLForResource:@"AppleIncRootCertificate" withExtension:@"cer"];
    
    ReceiptVerifier *verifier = [ReceiptVerifier verifierFromCertFile:appleCert.path];
    AppStoreReceipt *receipt = [AppStoreReceipt parseFromFile:receiptURL.path];
    
    verifier.delegate = self;
    [verifier verify:receipt];
}

- (void)showPurchaseView
{
    [_purchaseView removeFromSuperview];
    [_thanksView removeFromSuperview];
    
    [_additionalViewContainer addSubview:_purchaseView];
    
    _purchaseButton.enabled = NO;
    _restorePurchaseButton.enabled = NO;
    
    if (![SKPaymentQueue canMakePayments]) {
        _priceLabel.stringValue = NSLocalizedString(@"Preference_PurchaseCannotMakePayments", @"Sorry, you are not allowed to make payments.");
    }
    else {
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

// TODO remove
extern BOOL __fakePurchased__;

- (IBAction)purchasePressed:(id)sender
{
    // TODO
    __fakePurchased__ = YES;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [[IAP sharedInstance] notifyPurchased:kIAPProductFullVersion];
    });
}

- (IBAction)restorePurchasePressed:(id)sender
{
    // TODO
    __fakePurchased__ = YES;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [[IAP sharedInstance] notifyPurchased:kIAPProductFullVersion];
    });
}

- (IBAction)tweetPressed:(id)sender
{
    NSString *text = NSLocalizedString(@"Preference_PurchaseTweetMessage", @"NASequencer, the text-based music composition App.");
    NSString *encodedText = [text stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet alphanumericCharacterSet]];
    NSString *encodedUrl = [@"https://nasequencer.com" stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet alphanumericCharacterSet]];
    NSURL *url = [NSURL URLWithString:[NSString stringWithFormat:@"https://twitter.com/intent/tweet?text=%@&url=%@&related=nasequencer", encodedText, encodedUrl]];
    [[NSWorkspace sharedWorkspace] openURL: url];
}

- (IBAction)shareOnFacebookPressed:(id)sender
{
    NSString *encodedUrl = [@"https://nasequencer.com" stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet alphanumericCharacterSet]];
    NSURL *url = [NSURL URLWithString:[NSString stringWithFormat:@"https://www.facebook.com/sharer/sharer.php?u=%@", encodedUrl]];
    [[NSWorkspace sharedWorkspace] openURL: url];
}

#pragma mark ReceiptVerifierDelegate

- (void)verifierDidVerifySuccess:(ReceiptVerifier*)verifier
{
    [verifier findIAPProduct:kIAPProductFullVersion];
}

- (void)verifierDidVerifyFail:(ReceiptVerifier*)verifier
{
    [self showPurchaseView];
}

- (void)verifier:(ReceiptVerifier*)verifier didIAPProductFound:(NSString *)productID quantity:(int)quantity
{
    [self showThanksView];
}

- (void)verifier:(ReceiptVerifier*)verifier didIAPProductNotFound:(NSString *)productID
{
    [self showPurchaseView];
}

#pragma mark IAPObserver

- (void)iap:(IAP *)iap didPurchaseProduct:(NSString *)productId
{
    [self verifyReceipt];;
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
    
    self.state = NSOnState;
}

- (void)mouseUp:(NSEvent *)theEvent
{
    if (!self.enabled) {
        return;
    }
    
    self.state = NSOffState;
    
    [self sendAction:self.action to:self.target];
}

- (void)mouseExited:(NSEvent *)theEvent
{
    if (!self.enabled) {
        return;
    }
    
    self.state = NSOffState;
}

@end
