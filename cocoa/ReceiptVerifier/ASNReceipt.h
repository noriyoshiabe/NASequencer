//
//  ASNReceipt.h
//  ReceiptVerifier
//
//  Created by abechan on 2016/04/27.
//  Copyright © 2016年 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ASNReceipt : NSObject
- (void)parseReceipt:(const uint8_t *)p end:(const uint8_t *)end;
- (void)parseASNSet:(const uint8_t *)p end:(const uint8_t *)end;
- (void)parseASNSequence:(const uint8_t *)p end:(const uint8_t *)end;
- (void)parseASNAttrType:(int)attrType p:(const uint8_t *)p end:(const uint8_t *)end length:(long)length;
- (NSString *)parseASNString:(const uint8_t *)p end:(const uint8_t *)end;
- (int)parseASNInteger:(const uint8_t *)p end:(const uint8_t *)end;
@end
