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

#ifdef __IAP_MOCK__
#import "IAPMock.h"
#endif

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
    
    if (![SKPaymentQueue canMakePayments]) {
        _priceLabel.stringValue = NSLocalizedString(@"Purchase_CannotMakePayments", @"Sorry, you are not allowed to make payments.");
    }
    else {
        _priceLabel.stringValue = NSLocalizedString(@"Purchase_PriceLoading", @"Loading…");
        
        [[IAP sharedInstance] requestProductInfo:@[kIAPProductFullVersion] callback:^(SKProductsResponse *response) {
            for (SKProduct *product in response.products) {
                if ([product.productIdentifier isEqualToString:kIAPProductFullVersion]) {
                    NSNumberFormatter *formatter = [[NSNumberFormatter alloc] init];
                    formatter.locale = product.priceLocale;
                    formatter.numberStyle = NSNumberFormatterCurrencyISOCodeStyle;
                    formatter.formatterBehavior = NSNumberFormatterBehavior10_4;
                    
                    NSString *format = NSLocalizedString(@"Purchase_PriceFormat", @"%@, for all your Macs");
                    _priceLabel.stringValue = [NSString stringWithFormat:format, [formatter stringFromNumber:product.price]];
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
#ifdef __IAP_MOCK__
    [[IAP sharedInstance] paymentQueue:[SKPaymentQueue defaultQueue] updatedTransactions:@[[[FakePaymentTransaction alloc] initWithState:SKPaymentTransactionStatePurchasing error:nil]]];
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"FakePurchased"];
        [[NSUserDefaults standardUserDefaults] synchronize];
    
        [[IAP sharedInstance] paymentQueue:[SKPaymentQueue defaultQueue] updatedTransactions:@[[[FakePaymentTransaction alloc] initWithState:SKPaymentTransactionStatePurchased error:[NSError errorWithDomain:SKErrorDomain code:SKErrorPaymentInvalid userInfo:nil]]]];
    });
#else
    [[IAP sharedInstance] purchase:kIAPProductFullVersion];
#endif // __IAP_MOCK__
}

- (IBAction)restorePurchasePressed:(id)sender
{
#ifdef __IAP_MOCK__
    [[IAP sharedInstance] paymentQueue:[SKPaymentQueue defaultQueue] updatedTransactions:@[[[FakePaymentTransaction alloc] initWithState:SKPaymentTransactionStatePurchasing error:nil]]];
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"FakePurchased"];
        [[NSUserDefaults standardUserDefaults] synchronize];
        
        [[IAP sharedInstance] paymentQueue:[SKPaymentQueue defaultQueue] updatedTransactions:@[[[FakePaymentTransaction alloc] initWithState:SKPaymentTransactionStateRestored error:[NSError errorWithDomain:SKErrorDomain code:SKErrorPaymentInvalid userInfo:nil]]]];
    });
#else
    [[IAP sharedInstance] restorePurchase:kIAPProductFullVersion];
#endif // __IAP_MOCK__
}

- (IBAction)tweetPressed:(id)sender
{
    NSString *text = NSLocalizedString(@"Preference_PurchaseTweetMessage", @"NASequencer, the text-based MIDI composer. For Composition, Transcription, Karaoke, etc.");
    NSString *encodedText = [text stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet alphanumericCharacterSet]];
    NSString *encodedUrl = [@"https://nasequencer.com" stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet alphanumericCharacterSet]];
    NSURL *url = [NSURL URLWithString:[NSString stringWithFormat:@"https://twitter.com/intent/tweet?text=%@&url=%@", encodedText, encodedUrl]];
    [[NSWorkspace sharedWorkspace] openURL: url];
}

- (IBAction)shareOnFacebookPressed:(id)sender
{
    NSString *encodedUrl = [@"https://nasequencer.com" stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet alphanumericCharacterSet]];
    NSURL *url = [NSURL URLWithString:[NSString stringWithFormat:@"https://www.facebook.com/sharer/sharer.php?u=%@", encodedUrl]];
    [[NSWorkspace sharedWorkspace] openURL: url];
}

#pragma mark IAPObserver

- (void)iap:(IAP *)iap didUpdateTransaction:(SKPaymentTransaction *)transaction
{
    switch (transaction.transactionState) {
        case SKPaymentTransactionStatePurchasing:
            _purchaseButton.enabled = NO;
            _restorePurchaseButton.enabled = NO;
            break;
        case SKPaymentTransactionStateDeferred:
            break;
        case SKPaymentTransactionStateFailed:
            _purchaseButton.enabled = YES;
            _restorePurchaseButton.enabled = YES;
            break;
        case SKPaymentTransactionStatePurchased:
        case SKPaymentTransactionStateRestored:
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
