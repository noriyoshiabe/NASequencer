//
//  IAP.m
//  NAMIDI
//
//  Created by abechan on 4/28/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "IAP.h"
#import "IAPDelegate_OS15.h"
#import "IAPDelegate_OS26.h"

@implementation IAPEntitlement
@end

@implementation IAPProductInfo
@end

@implementation IAPTransaction
@end

@interface IAP () {
    id<IAPDelegate> _delegate;
}
@end

@implementation IAP

static id<IAPDelegate> _sharedInstance = nil;

+ (id<IAPDelegate>)sharedInstance
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if (@available(macOS 26.0, *)) {
            _sharedInstance = [[IAPDelegate_OS26 alloc] init];
        } else {
            _sharedInstance = [[IAPDelegate_OS15 alloc] init];
        }
    });
    return _sharedInstance;
}

@end
