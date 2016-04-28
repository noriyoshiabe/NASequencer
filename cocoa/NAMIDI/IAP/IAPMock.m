//
//  IAPMock.m
//  NAMIDI
//
//  Created by abechan on 4/29/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "IAPMock.h"

@implementation FakePayment

- (NSString *)productIdentifier
{
    return kIAPProductFullVersion;
}

@end

@implementation FakePaymentTransaction

- (instancetype)initWithState:(SKPaymentTransactionState)state error:(NSError *)error
{
    self = [super init];
    if (self) {
        __state = state;
        __error = error;
    }
    return self;
}

- (SKPaymentTransactionState)transactionState
{
    return __state;
}

- (NSError *)error
{
    return __error;
}

- (SKPayment *)payment
{
    return [[FakePayment alloc] init];
}

@end

@implementation FakeProductResponse

- (NSArray<SKProduct *> *)products
{
    return @[[[FakeProduct alloc] init]];
}

@end

@implementation FakeProduct

- (NSString *)localizedTitle
{
    return @"Full Version";
}

- (NSDecimalNumber *)price
{
    return [NSDecimalNumber decimalNumberWithString:@"24.99"];
}

- (NSLocale *)priceLocale
{
    return [NSLocale localeWithLocaleIdentifier:@"en_US"];
}

- (NSString *)productIdentifier
{
    return kIAPProductFullVersion;
}

@end