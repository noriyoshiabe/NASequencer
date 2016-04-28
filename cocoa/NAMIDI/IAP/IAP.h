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
- (void)iap:(IAP *)iap didPurchaseProduct:(NSString *)productId;
@end

@interface IAP : NSObject
+ (IAP *)sharedInstance;
- (void)initialize;
- (void)finalize;
- (void)addObserver:(id<IAPObserver>)observer;
- (void)removeObserver:(id<IAPObserver>)observer;
- (void)requestProductInfo:(NSArray *)productIdentifiers callback:(void (^)(SKProductsResponse *response))callback;
#ifdef DEBUG
- (void)notifyPurchased:(NSString *)productID;
#endif
@end
