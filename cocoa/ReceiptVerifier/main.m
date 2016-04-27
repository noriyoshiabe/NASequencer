//
//  main.m
//  ReceiptVerifier
//
//  Created by abechan on 2016/04/27.
//  Copyright © 2016年 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "AppStoreReceipt.h"

int main(int argc, const char * argv[]) {
    @autoreleasepool {
        AppStoreReceipt *receipt = [AppStoreReceipt parseFromFile:@"/Users/abechan/workspace/namidi/cocoa/openssl/receipt"];
        NSLog(@"%@", receipt);
    }
    return 0;
}
