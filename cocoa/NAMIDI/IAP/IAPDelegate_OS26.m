//
//  IAPDelegate_OS26.m
//  NAMIDI
//
//  Created by abechan on 2026/01/11.
//  Copyright © 2026 Noriyoshi Abe. All rights reserved.
//

#import "IAPDelegate_OS26.h"
#import "NASequencer-Swift.h"
#import "ObserverList.h"

NS_ASSUME_NONNULL_BEGIN

@interface IAPDelegate_OS26 () {
    ObserverList *_observers;
}
@end

@implementation IAPDelegate_OS26

- (instancetype)init
{
    self = [super init];
    if (self) {
        _observers = [[ObserverList alloc] init];
    }
    return self;
}

- (void)addObserver:(id<IAPObserver>)observer
{
    [_observers addObserver:observer];
}

- (void)removeObserver:(id<IAPObserver>)observer
{
    [_observers removeObserver:observer];
}

- (void)initialize {
    [IAPDelegate_OS26_Swift transactionUpdatesWithCompletionHandler:^(IAPTransaction * _Nullable transaction) {
        for (id<IAPObserver> observer in _observers) {
            [observer iap:self didUpdateTransaction:transaction];
        }
    }];
}

- (void)findIAPProduct:(NSString *)productID found:(void (^)(NSString *, int))found notFound:(void (^)(NSString *))notFound {
    [IAPDelegate_OS26_Swift latestEntitlementFor:productID completionHandler:^(IAPEntitlement * _Nullable entitlement) {
        [NSThread performBlockOnMainThread:^{
            if (entitlement) {
                if (found != nil) {
                    found(entitlement.productId, entitlement.quantity);
                }
            } else {
                if (notFound != nil) {
                    notFound(productID);
                }
            }
        }];
    }];
}

- (void)purchase:(NSString *)productID { 
    [IAPDelegate_OS26_Swift purchaseWithProductId:productID completionHandler:^(IAPTransaction * _Nonnull transaction) {
        [NSThread performBlockOnMainThread:^{
            for (id<IAPObserver> observer in _observers) {
                [observer iap:self didUpdateTransaction:transaction];
            }
        }];
    }];
}

- (void)requestProductInfo:(NSArray *)productIdentifiers callback:(void (^)(NSArray *))callback {
    [IAPDelegate_OS26_Swift productsFor:productIdentifiers completionHandler:^(NSArray<IAPProductInfo *> * _Nonnull productInfos) {
        [NSThread performBlockOnMainThread:^{
            callback(productInfos);
        }];
    }];
}

- (void)restorePurchase:(NSString *)productID { 
    [IAPDelegate_OS26_Swift appStoreSyncWithCompletionHandler:^{
        [NSThread performBlockOnMainThread:^{
            IAPTransaction *transaction = [[IAPTransaction alloc] init];
            transaction.productId = productID;
            transaction.transactionState = IAPTransactionStateRestored;
            
            for (id<IAPObserver> observer in _observers) {
                [observer iap:self didUpdateTransaction:transaction];
            }
        }];
    }];
}

- (BOOL)canMakePayments
{
    return IAPDelegate_OS26_Swift.canMakePayments;
}

@end

NS_ASSUME_NONNULL_END
