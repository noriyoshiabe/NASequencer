//
//  ErrorMessage.h
//  NAMIDI
//
//  Created by abechan on 2015/05/09.
//  Copyright (c) 2015年 abechan. All rights reserved.
//

#ifndef NAMIDI_ErrorMessage_h
#define NAMIDI_ErrorMessage_h

#import "ParseError.h"

@interface ErrorMessage : NSObject
+ (NSString *)parseError:(ParseError)error info:(const void *)info;
@end

#endif
