//
//  IAPMock.h
//  NAMIDI
//
//  Created by abechan on 4/29/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//


#ifdef __IAP_MOCK__

#import <Foundation/Foundation.h>
#import <StoreKit/StoreKit.h>
#import "IAP.h"

@interface FakePayment : SKPayment
@end

@interface FakePaymentTransaction : SKPaymentTransaction {
    SKPaymentTransactionState __state;
    NSError *__error;
}
- (instancetype)initWithState:(SKPaymentTransactionState)state error:(NSError *)error;
@end

@interface FakeProductResponse : SKProductsResponse
@end

@interface FakeProduct : SKProduct
@end

#endif // __IAP_MOCK__