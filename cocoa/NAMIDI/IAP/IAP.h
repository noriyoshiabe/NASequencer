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

@interface IAPEntitlement : NSObject
@property (strong, nonatomic) NSString *productId;
@property (nonatomic) int quantity;
@end

@interface IAPProductInfo : NSObject
@property (strong, nonatomic) NSString *productId;
@property (strong, nonatomic) NSString *displayPrice;
@end

typedef NS_ENUM(NSInteger, IAPTransactionState) {
    IAPTransactionStatePurchasing = SKPaymentTransactionStatePurchasing,
    IAPTransactionStatePurchased = SKPaymentTransactionStatePurchased,
    IAPTransactionStateFailed = SKPaymentTransactionStateFailed,
    IAPTransactionStateRestored = SKPaymentTransactionStateRestored,
    IAPTransactionStateDeferred = SKPaymentTransactionStateDeferred,
    
    IAPTransactionStateUnknown = -1,
    IAPTransactionStateCanceled = -2,
};

@interface IAPTransaction : NSObject
@property (strong, nonatomic) NSString *productId;
@property (nonatomic) IAPTransactionState transactionState;
@property (strong, nonatomic) NSError *error;
@end

@protocol IAPDelegate;
@protocol IAPObserver <NSObject>
- (void)iap:(id<IAPDelegate>)iap didUpdateTransaction:(IAPTransaction *)transaction;
@end

@protocol IAPDelegate <NSObject>
- (void)initialize;
- (void)finalize;
- (void)addObserver:(id<IAPObserver>)observer;
- (void)removeObserver:(id<IAPObserver>)observer;
- (void)findIAPProduct:(NSString *)productID found:(void(^)(NSString *productID, int quantity))found notFound:(void(^)(NSString *productID))notFound;
- (void)requestProductInfo:(NSArray *)productIdentifiers callback:(void (^)(NSArray *productInfos))callback;
- (void)purchase:(NSString *)productID;
- (void)restorePurchase:(NSString *)productID;
- (BOOL)canMakePayments;
@end

@interface IAP : NSObject
+ (id<IAPDelegate>)sharedInstance;
@end
