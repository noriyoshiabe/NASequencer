//
//  main.m
//  ReceiptVerifier
//
//  Created by abechan on 2016/04/27.
//  Copyright © 2016年 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "ReceiptVerifier.h"
#import "AppStoreReceipt.h"

@interface TestDriver : NSObject <ReceiptVerifierDelegate>
@end

@implementation TestDriver

- (void)verifierDidVerifySuccess:(ReceiptVerifier *)verifier
{
    NSLog(@"%s", __func__);
    [verifier findIAPProduct:@"test_product_id"];
}

- (void)verifierDidVerifyFail:(ReceiptVerifier *)verifier
{
    NSLog(@"%s", __func__);
}

- (void)verifier:(ReceiptVerifier *)verifier didIAPProductFound:(NSString *)productID quantity:(int)quantity
{
    NSLog(@"%s", __func__);
    NSLog(@"%@: %d", productID, quantity);
}

- (void)verifier:(ReceiptVerifier *)verifier didIAPProductNotFound:(NSString *)productID
{
    NSLog(@"%s", __func__);
    NSLog(@"%@", productID);
}

@end

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        TestDriver *driver = [[TestDriver alloc] init];
        
        NSDate *start = [NSDate date];
        
        ReceiptVerifier *verifier = [ReceiptVerifier verifierFromCertFile:@"/Users/abechan/workspace/namidi/cocoa/openssl/AppleIncRootCertificate.cer"];
        AppStoreReceipt *receipt = [AppStoreReceipt parseFromFile:@"/Users/abechan/workspace/namidi/cocoa/openssl/receipt"];
        
        NSLog(@"%@", receipt);
        
        verifier.delegate = driver;
        [verifier verify:receipt];
        
        NSLog(@"time elapsed: %lf", -[start timeIntervalSinceNow]);
    }
    return 0;
}
