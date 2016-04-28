//
//  IAP.h
//  NAMIDI
//
//  Created by abechan on 4/28/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>

#define kIAPProductFullVersion @"com.nasequencer.NASequencer.full_version"

@class IAP;
@protocol IAPObserver <NSObject>
- (void)iap:(IAP *)iap didUpdateTransaction:(SKPaymentTransaction *)transaction;
@end

@interface IAP : NSObject
@property (readonly, nonatomic) BOOL hasFullVersion;
+ (IAP *)sharedInstance;
- (void)initialize;
- (void)finalize;
- (void)addObserver:(id<IAPObserver>)observer;
- (void)removeObserver:(id<IAPObserver>)observer;
- (void)requestProductInfo:(NSArray *)productIdentifiers callback:(void (^)(SKProductsResponse *response))callback;
- (void)finishTransaction:(SKPaymentTransaction *)transaction;
// TODO remove
#ifdef DEBUG
- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray <SKPaymentTransaction *> *)transactions;
#endif
@end
