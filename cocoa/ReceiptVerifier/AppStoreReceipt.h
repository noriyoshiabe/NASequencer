//
//  AppStoreReceipt.h
//  ReceiptVerifier
//
//  Created by abechan on 2016/04/27.
//  Copyright © 2016年 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <openssl/pkcs7.h>
#import "ASNReceipt.h"

@interface AppStoreReceipt : ASNReceipt
+ (AppStoreReceipt *)parseFromFile:(NSString *)filepath;
@property (readonly, nonatomic) PKCS7 *p7;
@property (readonly, nonatomic) NSString *bundleIdentifier;
@property (readonly, nonatomic) NSData *bundleIdHash;
@property (readonly, nonatomic) NSString *bundleShortVersion;
@property (readonly, nonatomic) NSData *opaque;
@property (readonly, nonatomic) NSData *sha1;
@property (readonly, nonatomic) NSString *originalAppVersion;
@property (readonly, nonatomic) NSString *expirationDate;
@property (readonly, nonatomic) NSArray *iapReceipts;
@end
