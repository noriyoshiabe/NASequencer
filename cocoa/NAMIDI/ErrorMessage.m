//
//  ErrorMessage.m
//  NAMIDI
//
//  Created by abechan on 2015/05/09.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "ErrorMessage.h"

@implementation ErrorMessage

+ (NSString *)parseError:(ParseError)error info:(const void *)info
{
    NSString *key = [NSString stringWithUTF8String:ParseError2String(error)];
    return [[NSBundle mainBundle] localizedStringForKey:key value:nil table:@"ParseError"];
}

@end
