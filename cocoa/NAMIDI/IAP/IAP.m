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

#ifdef __IAP_MOCK__
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
    NSMutableDictionary *_products;
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
        _products = [NSMutableDictionary dictionary];
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
#ifdef __IAP_MOCK__
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        callback([[FakeProductResponse alloc] init]);
    });
#else
    SKProductsRequest *productsRequest = [[SKProductsRequest alloc] initWithProductIdentifiers:[NSSet setWithArray:productIdentifiers]];
    
    ProductRequestInfo *requestInfo = [[ProductRequestInfo alloc] init];
    requestInfo.request = productsRequest;
    requestInfo.callback = callback;
    
    [_productRequestInfos addObject:requestInfo];
    
    productsRequest.delegate = self;
    [productsRequest start];
#endif // __IAP_MOCK__
}

- (void)purchase:(NSString *)productID
{
    SKProduct *product = _products[productID];
    if (product) {
        SKPayment *payment = [SKPayment paymentWithProduct:product];
        [[SKPaymentQueue defaultQueue] addPayment:payment];
    }
}

- (void)restorePurchase:(NSString *)productID
{
    [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
}

#pragma mark SKProductsRequestDelegate

- (void)productsRequest:(SKProductsRequest *)request didReceiveResponse:(SKProductsResponse *)response
{
    for (SKProduct *product in response.products) {
        _products[product.productIdentifier] = product;
    }
    
    for (ProductRequestInfo *requestInfo in _productRequestInfos) {
        if ([request isEqualTo:requestInfo.request]) {
            [NSThread performBlockOnMainThread:^{
                requestInfo.callback(response);
            }];
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
                    break;
                case SKPaymentTransactionStateFailed:
                {
                    [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
                    
                    if (SKErrorPaymentCancelled != transaction.error.code) {
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
    [NSThread performBlockOnMainThread:^{
        NSAlert *alert = [[NSAlert alloc] init];
        alert.messageText = NSLocalizedString(@"Purchase_RestoreFailed", @"Restore Purchase Faild");
        
        NSString *errorText;
        if (error) {
            NSString *format = NSLocalizedString(@"Purchase_RestorePurchaseFaildErrorFormat", @"An error has occurred.\n%@ - %d");
            errorText = [NSString stringWithFormat:format, error.domain, error.code];
        }
        else {
            errorText = @"Unknown Error.";
        }
        
        NSString *informative = NSLocalizedString(@"Purchase_RestorePurchaseFaildInformative",
                                                  @"%@\n\n"
                                                  @"Please try again and make sure you're using the same Apple ID you have previously used to purchase.");
        
        alert.informativeText = [NSString stringWithFormat:informative, errorText];
        [alert addButtonWithTitle:NSLocalizedString(@"Close", @"Close")];;
        [alert runModal];
    }];
}

#pragma mark ReceiptVerifierDelegate

- (void)verifierDidVerifySuccess:(ReceiptVerifier*)verifier
{
    _purchasedTransaction = nil;
}

- (void)verifierDidVerifyFail:(ReceiptVerifier*)verifier
{
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
