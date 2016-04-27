//
//  ReceiptVerifier.h
//  ReceiptVerifier
//
//  Created by abechan on 2016/04/27.
//  Copyright © 2016年 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "AppStoreReceipt.h"

@class ReceiptVerifier;
@protocol ReceiptVerifierDelegate <NSObject>
- (void)verifierDidVerifySuccess:(ReceiptVerifier*)verifier;
- (void)verifierDidVerifyFail:(ReceiptVerifier*)verifier;
- (void)verifier:(ReceiptVerifier*)verifier didIAPProductFound:(NSString *)productID quantity:(int)quantity;
- (void)verifier:(ReceiptVerifier*)verifier didIAPProductNotFound:(NSString *)productID;
@end

@interface ReceiptVerifier : NSObject
@property (weak, nonatomic) id<ReceiptVerifierDelegate> delegate;
+ (ReceiptVerifier *)verifierFromCertFile:(NSString *)filepath;
- (void)verify:(AppStoreReceipt *)receipt;
- (void)findIAPProduct:(NSString *)productID;
@end
