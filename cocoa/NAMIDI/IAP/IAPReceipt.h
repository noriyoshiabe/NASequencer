//
//  IAPReceipt.h
//  NAMIDI
//
//  Created by abechan on 2016/04/27.
//  Copyright © 2016年 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ASNReceipt.h"

@interface IAPReceipt : ASNReceipt
- (instancetype)initWithReceipt:(const uint8_t *)p end:(const uint8_t *)end;
@property (readonly, nonatomic) int quantity;
@property (readonly, nonatomic) NSString *productId;
@property (readonly, nonatomic) NSString *transactionId;
@property (readonly, nonatomic) NSString *purchaseDate;
@property (readonly, nonatomic) NSString *originalTransactionId;
@property (readonly, nonatomic) NSString *originalPurchaseDate;
@property (readonly, nonatomic) NSString *subscriptionExpirationDate;
@property (readonly, nonatomic) int webOrderLineItemID;
@property (readonly, nonatomic) NSString *cancelDate;
@end
