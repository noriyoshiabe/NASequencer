//
//  IAP.m
//  NAMIDI
//
//  Created by abechan on 4/28/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "IAP.h"
#import "ObserverList.h"
#import "ReceiptVerifier.h"

#import <Crashlytics/Answers.h>

// TODO remove
#ifdef DEBUG
#import "IAPMock.h"
#endif

@interface ProductRequestInfo : NSObject
@property (strong, nonatomic) SKProductsRequest *request;
@property (strong, nonatomic) void (^callback)(SKProductsResponse *response);
@end

@implementation ProductRequestInfo
@end

@interface IAP () <SKProductsRequestDelegate, SKPaymentTransactionObserver, ReceiptVerifierDelegate> {
    NSMutableArray *_productRequestInfos;
    ObserverList *_observers;
    ReceiptVerifier *_verifier;
    SKPaymentTransaction *_purchasedTransaction;
    void(^_found)(NSString *, int);
    void(^_notFound)(NSString *);
}
@end

@implementation IAP

static IAP *_sharedInstance = nil;

+ (IAP *)sharedInstance
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _sharedInstance = [[IAP alloc] init];
    });
    return _sharedInstance;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _productRequestInfos = [NSMutableArray array];
        _observers = [[ObserverList alloc] init];
        
        NSURL *appleCert = [[NSBundle mainBundle] URLForResource:@"AppleIncRootCertificate" withExtension:@"cer"];
        _verifier = [ReceiptVerifier verifierFromCertFile:appleCert.path];
        _verifier.delegate = self;
    }
    return self;
}

- (void)initialize
{
    [self verifyReceipt:self];
    [[SKPaymentQueue defaultQueue] addTransactionObserver:self];
}

- (void)finalize
{
    [[SKPaymentQueue defaultQueue] removeTransactionObserver:self];
}

- (void)addObserver:(id<IAPObserver>)observer
{
    [_observers addObserver:observer];
}

- (void)removeObserver:(id<IAPObserver>)observer
{
    [_observers removeObserver:observer];
}

- (void)verifyReceipt:(id<ReceiptVerifierDelegate>)delegate
{
    NSURL *receiptURL = [[NSBundle mainBundle] appStoreReceiptURL];
    AppStoreReceipt *receipt = [AppStoreReceipt parseFromFile:receiptURL.path];
    [_verifier verify:receipt];
}

- (void)findIAPProduct:(NSString *)productID found:(void(^)(NSString *productID, int quantity))found notFound:(void(^)(NSString *productID))notFound
{
    _found = found;
    _notFound = notFound;
    [_verifier findIAPProduct:productID];
}

- (void)requestProductInfo:(NSArray *)productIdentifiers callback:(void (^)(SKProductsResponse *response))callback
{
    // TODO test after IAP product is enabled
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        callback([[FakeProductResponse alloc] init]);
    });
    
#if 0
    SKProductsRequest *productsRequest = [[SKProductsRequest alloc] initWithProductIdentifiers:[NSSet setWithArray:productIdentifiers]];
    
    ProductRequestInfo *requestInfo = [[ProductRequestInfo alloc] init];
    requestInfo.request = productsRequest;
    requestInfo.callback = callback;
    
    [_productRequestInfos addObject:requestInfo];
    
    productsRequest.delegate = self;
    [productsRequest start];
#endif
}

#pragma mark SKProductsRequestDelegate

- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response
{
    for (ProductRequestInfo *requestInfo in _productRequestInfos) {
        if ([request isEqualTo:requestInfo.request]) {
            requestInfo.callback(response);
            [_productRequestInfos removeObject:requestInfo];
            break;
        }
    }
    
    for (NSString *invalidIdentifier in response.invalidProductIdentifiers) {
        NSLog(@"invalid product identifier: %@", invalidIdentifier);
    }
}

#pragma mark SKPaymentTransactionObserver

- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray <SKPaymentTransaction *> *)transactions
{
    [NSThread performBlockOnMainThread:^{
        for (SKPaymentTransaction *transaction in transactions) {
            switch (transaction.transactionState) {
                case SKPaymentTransactionStatePurchasing:
                    break;
                case SKPaymentTransactionStateDeferred:
                    [Answers logCustomEventWithName:@"Trace" customAttributes:@{@"Category": @"Purchase", @"Message" : @"SKPaymentTransactionStateDeferred"}];
                    break;
                case SKPaymentTransactionStateFailed:
                {
                    [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
                    
                    if (SKErrorPaymentCancelled != transaction.error.code) {
                        [Answers logCustomEventWithName:@"Trace"
                                       customAttributes:@{@"Category": @"Purchase",
                                                          @"Message" : [NSString stringWithFormat:@"SKPaymentTransactionStateFailed %@", transaction.error]}];
                        
                        NSString *informative = NSLocalizedString(@"Purchase_PurchaseFaildInformative", @"An error has occurred.\n%@ - %d");
                        NSAlert *alert = [[NSAlert alloc] init];
                        alert.messageText = NSLocalizedString(@"Purchase_PurshaseFailed", @"Purchase Faild");
                        alert.informativeText = [NSString stringWithFormat:informative, transaction.error.domain, transaction.error.code];
                        [alert addButtonWithTitle:NSLocalizedString(@"Close", @"Close")];;
                        [alert runModal];
                    }
                }
                    break;
                case SKPaymentTransactionStatePurchased:
                case SKPaymentTransactionStateRestored:
                    _purchasedTransaction = transaction;
                    [self verifyReceipt:self];
                    [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
                    break;
            }
            
            for (id<IAPObserver> observer in _observers) {
                [observer iap:self didUpdateTransaction:transaction];
            }
        }
    }];
}

- (void)paymentQueue:(SKPaymentQueue *)queue restoreCompletedTransactionsFailedWithError:(NSError *)error
{
    [Answers logCustomEventWithName:@"Trace"
                   customAttributes:@{@"Category": @"Purchase",
                                      @"Message" : [NSString stringWithFormat:@"paymentQueue:restoreCompletedTransactionsFailedWithError: %@", error]}];
    
    NSString *informative = NSLocalizedString(@"Purchase_RestorePurchaseFaildInformative", @"An error has occurred.\n%@ - %d");
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = NSLocalizedString(@"Purchase_RestoreFailed", @"Restore Purchase Faild");
    alert.informativeText = [NSString stringWithFormat:informative, error.domain, error.code];
    [alert addButtonWithTitle:NSLocalizedString(@"Close", @"Close")];;
    [alert runModal];
}

#pragma mark ReceiptVerifierDelegate

- (void)verifierDidVerifySuccess:(ReceiptVerifier*)verifier
{
    if (_purchasedTransaction) {
        [self findIAPProduct:_purchasedTransaction.payment.productIdentifier found:^(NSString *productID, int quantity) {
            SKPaymentTransaction *transaction = _purchasedTransaction;
            
            [self requestProductInfo:@[transaction.payment.productIdentifier] callback:^(SKProductsResponse *response) {
                SKProduct *product = response.products.firstObject;
                if (product) {
                    if (SKPaymentTransactionStatePurchased == transaction.transactionState) {
                        NSDictionary *types = @{kIAPProductFullVersion: @"Non-consumable"};
                        [Answers logPurchaseWithPrice:product.price
                                             currency:[product.priceLocale objectForKey:NSLocaleCurrencyCode]
                                              success:@YES
                                             itemName:product.localizedTitle
                                             itemType:types[product.productIdentifier]
                                               itemId:product.productIdentifier
                                     customAttributes:@{}];
                    }
                    else if (SKPaymentTransactionStateRestored == transaction.transactionState) {
                        [Answers logCustomEventWithName:@"Restore Purchase" customAttributes:@{@"Item ID" : product.productIdentifier}];
                    }
                    else {
                        [Answers logCustomEventWithName:@"Trace" customAttributes:@{@"Category": @"Purchase", @"Message" : [NSString stringWithFormat:@"Illegal transaction state with purchase success transactionState=%ld", transaction.transactionState]}];
                    }
                    
                }
            }];
            
            _purchasedTransaction = nil;
        } notFound:^(NSString *productID) {
            [Answers logCustomEventWithName:@"Trace" customAttributes:@{@"Category": @"Purchase", @"Message" : [NSString stringWithFormat:@"verifier:didIAPProductNotFound: productID=%@", productID]}];
            _purchasedTransaction = nil;
        }];
    }
}

- (void)verifierDidVerifyFail:(ReceiptVerifier*)verifier
{
    [Answers logCustomEventWithName:@"Trace" customAttributes:@{@"Category": @"Purchase", @"Message" : @"verifierDidVerifyFail:"}];
    _purchasedTransaction = nil;
}

- (void)verifier:(ReceiptVerifier*)verifier didIAPProductFound:(NSString *)productID quantity:(int)quantity
{
    if(_found) {
        _found(productID, quantity);
    }
    
    _found = nil;
    _notFound = nil;
}

- (void)verifier:(ReceiptVerifier*)verifier didIAPProductNotFound:(NSString *)productID
{
    if (_notFound) {
        _notFound(productID);
    }
    
    _found = nil;
    _notFound = nil;
}

@end