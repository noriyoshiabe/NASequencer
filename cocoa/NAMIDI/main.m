//
//  main.m
//  NAMIDI
//
//  Created by abechan on 2/8/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

int main(int argc, const char * argv[])
{
    return NSApplicationMain(argc, argv);
}


#import "ReceiptVerifier.h"
#import "AppStoreReceipt.h"

@interface ApplicationReceiptVerier : NSObject<ReceiptVerifierDelegate>
@end

@implementation ApplicationReceiptVerier

- (void)verifierDidVerifySuccess:(ReceiptVerifier*)verifier
{
}

- (void)verifierDidVerifyFail:(ReceiptVerifier*)verifier
{
    exit(173);
}

- (void)verifier:(ReceiptVerifier*)verifier didIAPProductFound:(NSString *)productID quantity:(int)quantity
{
}

- (void)verifier:(ReceiptVerifier*)verifier didIAPProductNotFound:(NSString *)productID
{
}

@end


__attribute__((constructor))
static void verifyReceipt()
{
    NSURL *receiptURL = [[NSBundle mainBundle] appStoreReceiptURL];
    NSURL *appleCert = [[NSBundle mainBundle] URLForResource:@"AppleIncRootCertificate" withExtension:@"cer"];
    
    ReceiptVerifier *verifier = [ReceiptVerifier verifierFromCertFile:appleCert.path];
    AppStoreReceipt *receipt = [AppStoreReceipt parseFromFile:receiptURL.path];

#if 0
    NSLog(@"AppStoreReceipt: %@", receipt);
#endif

    ApplicationReceiptVerier *appVerifier = [[ApplicationReceiptVerier alloc] init];
    
    verifier.delegate = appVerifier;
    [verifier verify:receipt];
}
