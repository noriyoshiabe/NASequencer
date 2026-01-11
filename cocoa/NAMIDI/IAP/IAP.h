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
+ (IAP *)sharedInstance;
- (void)initialize;
- (void)finalize;
- (void)addObserver:(id<IAPObserver>)observer;
- (void)removeObserver:(id<IAPObserver>)observer;
- (void)findIAPProduct:(NSString *)productID found:(void(^)(NSString *productID, int quantity))found notFound:(void(^)(NSString *productID))notFound;
- (void)requestProductInfo:(NSArray *)productIdentifiers callback:(void (^)(SKProductsResponse *response))callback;
- (void)purchase:(NSString *)productID;
- (void)restorePurchase:(NSString *)productID;
@end
