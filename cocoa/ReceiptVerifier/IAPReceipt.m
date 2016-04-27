//
//  IAPReceipt.m
//  ReceiptVerifier
//
//  Created by abechan on 2016/04/27.
//  Copyright © 2016年 Noriyoshi Abe. All rights reserved.
//

#import "IAPReceipt.h"

@implementation IAPReceipt

- (instancetype)initWithReceipt:(const uint8_t *)p end:(const uint8_t *)end
{
    self = [super init];
    if (self) {
        [self parseReceipt:p end:end];
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:
            @"<IAPReceipt %p "
            @"quantity: %d, "
            @"productId: %@, "
            @"transactionId: %@, "
            @"purchaseDate: %@, "
            @"originalTransactionId: %@, "
            @"originalPurchaseDate: %@, "
            @"subscriptionExpirationDate: %@, "
            @"webOrderLineItemID: %d, "
            @"cancelDate: %@"
            @">"
            ,self
            ,_quantity
            ,_productId
            ,_transactionId
            ,_purchaseDate
            ,_originalTransactionId
            ,_originalPurchaseDate
            ,_subscriptionExpirationDate
            ,_webOrderLineItemID
            ,_cancelDate];
}

- (void)parseASNAttrType:(int)attrType p:(const uint8_t *)p end:(const uint8_t *)end length:(long)length
{
    switch (attrType) {
        case 1701: // In App purchase - Quantity
            _quantity = [self parseASNInteger:p end:end];
            break;
        case 1702: // In App purchase - Product Id
            _productId = [self parseASNString:p end:end];
            break;
        case 1703: // In App purchase - Transaction Id
            _transactionId = [self parseASNString:p end:end];
            break;
        case 1704: // In App purchase - Purchase Date
            _purchaseDate = [self parseASNString:p end:end];
            break;
        case 1705: // In App purchase - Original Transaction Id
            _originalTransactionId = [self parseASNString:p end:end];
            break;
        case 1706: // In App purchase - Original Purchase Date
            _originalPurchaseDate = [self parseASNString:p end:end];
            break;
        case 1708: // In App purchase - Subscription Expiration Date
            _subscriptionExpirationDate = [self parseASNString:p end:end];
            break;
        case 1711: // In App purchase - Web Order Line Item ID
            _webOrderLineItemID = [self parseASNInteger:p end:end];
            break;
        case 1712: // In App purchase - Cancel Date
            _cancelDate = [self parseASNString:p end:end];
            break;
    }
}

@end
