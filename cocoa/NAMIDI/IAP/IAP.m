//
//  IAP.m
//  NAMIDI
//
//  Created by abechan on 4/28/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "IAP.h"
#import "IAPDelegate_OS15.h"

@implementation IAPProductInfo

- (instancetype)initWithProductId:(NSString *)productId displayPrice:(NSString *)displayPrice
{
    self = [super init];
    if (self) {
        _productId = productId;
        _displayPrice = displayPrice;
    }
    return self;
}

@end

@implementation IAPTransaction

- (instancetype)initWithProductId:(NSString *)productId transactionState:(IAPTransactionState)transactionState
{
    self = [super init];
    if (self) {
        _productId = productId;
        _transactionState = transactionState;
    }
    return self;
}

@end

@interface IAP () {
    id<IAPDelegate> _delegate;
}
@end

@implementation IAP

static id<IAPDelegate> _sharedInstance = nil;

+ (id<IAPDelegate>)sharedInstance
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _sharedInstance = [[IAPDelegate_OS15 alloc] init];
    });
    return _sharedInstance;
}

@end
