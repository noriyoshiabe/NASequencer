//
//  AppStoreReceipt.m
//  ReceiptVerifier
//
//  Created by abechan on 2016/04/27.
//  Copyright © 2016年 Noriyoshi Abe. All rights reserved.
//

#import "AppStoreReceipt.h"
#import "IAPReceipt.h"

@interface AppStoreReceipt () {
    NSMutableArray *_iapReceipts;
}
@end

@implementation AppStoreReceipt

+ (AppStoreReceipt *)parseFromFile:(NSString *)filepath
{
    AppStoreReceipt *receipt = [[AppStoreReceipt alloc] init];
    
    NSData *receiptData = [NSData dataWithContentsOfFile:filepath];
    if (!receiptData) {
        return receipt;
    }
    
    BIO *b_receipt = BIO_new_mem_buf(receiptData.bytes, (int)receiptData.length);
    receipt->_p7 = d2i_PKCS7_bio(b_receipt, NULL);
    BIO_free(b_receipt);
    
    [receipt parsePKCS7];
    
    return receipt;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        _iapReceipts = [NSMutableArray array];
    }
    return self;
}

- (void)dealloc
{
    PKCS7_free(_p7);
}

- (NSString *)description
{
    return [NSString stringWithFormat:
            @"<AppStroeReceipt %p "
            @"bundleIdentifier: %@, "
            @"bundleIdHash: %@, "
            @"bundleShortVersion: %@, "
            @"opaque: %@, "
            @"sha1: %@, "
            @"originalAppVersion: %@, "
            @"expirationDate: %@, "
            @"iapReceipts: %@"
            @">"
            ,self
            ,_bundleIdentifier
            ,_bundleIdHash
            ,_bundleShortVersion
            ,_opaque
            ,_sha1
            ,_originalAppVersion
            ,_expirationDate
            ,_iapReceipts];
}

- (BOOL)exist
{
    return NULL != _p7;
}

- (void)parsePKCS7
{
    ASN1_OCTET_STRING *octets = _p7->d.sign->contents->d.data;
    [self parseReceipt:octets->data end:octets->data + octets->length];
}

- (void)parseASNAttrType:(int)attrType p:(const uint8_t *)p end:(const uint8_t *)end length:(long)length
{
    switch (attrType) {
        case 2: // CFBundleIdentifier
            _bundleIdHash = [NSData dataWithBytes:p length:length];
            _bundleIdentifier = [self parseASNString:p end:end];
            break;
        case 3: // CFBundleShortVersionString
            _bundleShortVersion = [self parseASNString:p end:end];
            break;
        case 4: // Opaque
            _opaque = [NSData dataWithBytes:p length:length];
            break;
        case 5: // SHA-1 Hash
            _sha1 = [NSData dataWithBytes:p length:length];
            break;
        case 19: // Origianl App Version
            _originalAppVersion = [self parseASNString:p end:end];
            break;
        case 21: // Expiration Date
            _expirationDate = [self parseASNString:p end:end];
            break;
        case 17: // In App purchase
            [_iapReceipts addObject:[[IAPReceipt alloc] initWithReceipt:p end:p + length]];
            break;
    }
}

@end
