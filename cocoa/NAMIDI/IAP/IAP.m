//
//  IAP.m
//  NAMIDI
//
//  Created by abechan on 4/28/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "IAP.h"
#import "ObserverList.h"

@interface ProductRequestInfo : NSObject
@property (strong, nonatomic) SKProductsRequest *request;
@property (strong, nonatomic) void (^callback)(SKProductsResponse *response);
@end

@implementation ProductRequestInfo
@end

@interface IAP () <SKProductsRequestDelegate, SKPaymentTransactionObserver> {
    NSMutableArray *_productRequestInfos;
    ObserverList *_observers;
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
    }
    return self;
}

- (void)initialize
{
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

- (void)requestProductInfo:(NSArray *)productIdentifiers callback:(void (^)(SKProductsResponse *response))callback
{
    // TODO test after IAP product is enabled
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        callback(nil);
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

- (void)finishTransaction:(SKPaymentTransaction *)transaction
{
    [[SKPaymentQueue defaultQueue] finishTransaction:transaction];
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
    for (SKPaymentTransaction *transaction in transactions) {
        [NSThread performBlockOnMainThread:^{
            for (id<IAPObserver> observer in _observers) {
                [observer iap:self didUpdateTransaction:transaction];
            }
        }];
    }
}

@end
