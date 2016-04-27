//
//  ASNReceipt.m
//  ReceiptVerifier
//
//  Created by abechan on 2016/04/27.
//  Copyright © 2016年 Noriyoshi Abe. All rights reserved.
//

#import "ASNReceipt.h"

#import <openssl/pkcs7.h>

@implementation ASNReceipt

- (void)parseReceipt:(const uint8_t *)p end:(const uint8_t *)end
{
    int type = 0;
    int xclass = 0;
    long length = 0;
    
    ASN1_get_object(&p, &length, &type, &xclass, end - p);
    if (V_ASN1_SET == type) {
        [self parseASNSet:p end:end];
    }
}

- (void)parseASNSet:(const uint8_t *)p end:(const uint8_t *)end
{
    int type = 0;
    int xclass = 0;
    long length = 0;
    
    while (p < end) {
        ASN1_get_object(&p, &length, &type, &xclass, end - p);
        if (V_ASN1_SEQUENCE == type) {
            [self parseASNSequence:p end:p + length];
        }
        p += length;
    }
}

- (void)parseASNSequence:(const uint8_t *)p end:(const uint8_t *)end
{
    int type = 0;
    int xclass = 0;
    long length = 0;
    
    while (p < end) {
        int attr_type = 0;
        int attr_version = 0;
        
        ASN1_get_object(&p, &length, &type, &xclass, end - p);
        if (type == V_ASN1_INTEGER) {
            if (1 == length) {
                attr_type = p[0];
            }
            else if (2 == length) {
                attr_type = (p[0] << 8) | p[1];
            }
        }
        p += length;
        
        ASN1_get_object(&p, &length, &type, &xclass, end - p);
        if (type == V_ASN1_INTEGER && 1 == length) {
            attr_version = p[0];
        }
        p += length;
        
        ASN1_get_object(&p, &length, &type, &xclass, end - p);
        [self parseASNAttrType:attr_type p:p end:end length:length];
        p += length;
    }
}

- (void)parseASNAttrType:(int)attrType p:(const uint8_t *)p end:(const uint8_t *)end length:(long)length
{
    
}

- (NSString *)parseASNString:(const uint8_t *)p end:(const uint8_t *)end
{
    int type = 0;
    int xclass = 0;
    long length = 0;
    
    ASN1_get_object(&p, &length, &type, &xclass, end - p);
    char *str = alloca(length + 1);
    strncpy(str, (const char *)p, length);
    str[length] = '\0';
    
    return [NSString stringWithUTF8String:str];
}

- (int)parseASNInteger:(const uint8_t *)p end:(const uint8_t *)end
{
    int type = 0;
    int xclass = 0;
    long length = 0;
    
    ASN1_get_object(&p, &length, &type, &xclass, end - p);
    
    int ret = 0;
    
    if (length) {
        ret |= p[0];
        if (1 < length) {
            ret |= p[1] << 8;
            if (2 < length) {
                ret |= p[2] << 16;
                if (3 < length) {
                    ret |= p[3] << 24;
                }
            }
        }
    }
    
    return ret;
}

@end
