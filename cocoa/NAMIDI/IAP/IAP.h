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
- (void)iap:(IAP *)iap didRefreshReceiptWithError:(NSError *)error;
@end

@interface IAP : NSObject
@property (readonly, nonatomic) BOOL hasFullVersion;
+ (IAP *)sharedInstance;
- (void)initialize;
- (void)finalize;
- (void)addObserver:(id<IAPObserver>)observer;
- (void)removeObserver:(id<IAPObserver>)observer;
- (void)findIAPProduct:(NSString *)productID found:(void(^)(NSString *productID, int quantity))found notFound:(void(^)(NSString *productID))notFound;
- (void)requestProductInfo:(NSArray *)productIdentifiers callback:(void (^)(SKProductsResponse *response))callback;
#ifdef __IAP_MOCK__
- (void)paymentQueue:(SKPaymentQueue *)queue updatedTransactions:(NSArray <SKPaymentTransaction *> *)transactions;
#endif
- (void)purchase:(NSString *)productID;
- (void)restorePurchase:(NSString *)productID;
@end
