//
//  ReceiptVerifier.m
//  NAMIDI
//
//  Created by abechan on 2016/04/27.
//  Copyright © 2016年 Noriyoshi Abe. All rights reserved.
//

#import "ReceiptVerifier.h"
#import "IAPReceipt.h"
#import "ReceiptVerifierHardCodedString.h"

#import <openssl/x509.h>
#import <openssl/err.h>

@interface ReceiptVerifier () {
    X509_STORE *_store;
    NSArray *_iapReceipts;
}
@end

@implementation ReceiptVerifier

+ (ReceiptVerifier *)verifierFromCertFile:(NSString *)filepath
{
    ReceiptVerifier *verifier = [[ReceiptVerifier alloc] init];
    
    NSData *x509Data = [NSData dataWithContentsOfFile:filepath];
    
    BIO *b_x509 = BIO_new_mem_buf(x509Data.bytes, (int)x509Data.length);
    
    X509_STORE *store = X509_STORE_new();
    X509 *appleCA = d2i_X509_bio(b_x509, NULL);
    X509_STORE_add_cert(store, appleCA);
    
    X509_free(appleCA);
    BIO_free(b_x509);

    verifier->_store = store;
    
    return verifier;
}

- (void)dealloc
{
    X509_STORE_free(_store);
}

- (void)verify:(AppStoreReceipt *)receipt
{
    if (!receipt.exist) {
        [_delegate verifierDidVerifyFail:self];
        return;
    }
    
    OpenSSL_add_all_digests();

    BIO *b_receiptPayload = BIO_new(BIO_s_mem());
    int result = PKCS7_verify(receipt.p7, NULL, _store, NULL, b_receiptPayload, 0);
    if (1 != result) {
        EVP_cleanup();
        BIO_free(b_receiptPayload);
        [_delegate verifierDidVerifyFail:self];
        return;
    }
    
    BIO_free(b_receiptPayload);
    
    if (![kReceiptVerifierHardCodedBundleIdentifier isEqualToString:receipt.bundleIdentifier]) {
        EVP_cleanup();
        [_delegate verifierDidVerifyFail:self];
        return;
    }
    
    if (![kReceiptVerifierHardCodedBundleShortVersion isEqualToString:receipt.bundleShortVersion]) {
        EVP_cleanup();
        [_delegate verifierDidVerifyFail:self];
        return;
    }
    
    NSData *macAddress = [self macAddress];
    
    NSMutableData *input = [NSMutableData data];
    [input appendData:macAddress];
    [input appendData:receipt.opaque];
    [input appendData:receipt.bundleIdHash];
    
    NSMutableData *hash = [NSMutableData dataWithLength:SHA_DIGEST_LENGTH];
    SHA1(input.bytes, input.length, hash.mutableBytes);
    
    if (![hash isEqualToData:receipt.sha1]) {
        EVP_cleanup();
        [_delegate verifierDidVerifyFail:self];
        return;
    }
    
    _iapReceipts = receipt.iapReceipts;
    
    EVP_cleanup();
    [_delegate verifierDidVerifySuccess:self];
}

#ifdef DEBUG
// TODO remove
BOOL __fakePurchased__ = NO;
#endif

- (void)findIAPProduct:(NSString *)productID
{
#ifdef DEBUG
    // TODO remove
    if (__fakePurchased__) {
        [_delegate verifier:self didIAPProductFound:productID quantity:1];
        return;
    }
#endif
    
    for (IAPReceipt *iapReceipt in _iapReceipts) {
        if ([iapReceipt.productId isEqualToString:productID]) {
            if (iapReceipt.originalTransactionId && iapReceipt.originalPurchaseDate) {
                if (!iapReceipt.cancelDate) {
                    if (0 < iapReceipt.quantity) {
                        [_delegate verifier:self didIAPProductFound:iapReceipt.productId quantity:iapReceipt.quantity];
                        return;
                    }
                }
            }
        }
    }
    
    [_delegate verifier:self didIAPProductNotFound:productID];
}

- (NSData *)macAddress
{
    kern_return_t kernResult;
    mach_port_t master_port;
    CFMutableDictionaryRef matchingDict;
    io_iterator_t iterator;
    io_object_t service;
    CFDataRef macAddress = NULL;
    kernResult = IOMasterPort(MACH_PORT_NULL, &master_port);
    
    if (kernResult != KERN_SUCCESS) {
        return NULL;
    }
    
    matchingDict = IOBSDNameMatching(master_port, 0, "en0");
    if (!matchingDict) {
        return NULL;
    }
    
    kernResult = IOServiceGetMatchingServices(master_port, matchingDict, &iterator);
    if (kernResult != KERN_SUCCESS) {
        return NULL;
    }
    
    while ((service = IOIteratorNext(iterator)) != 0) {
        io_object_t parentService;
        kernResult = IORegistryEntryGetParentEntry(service, kIOServicePlane, &parentService);
        if (kernResult == KERN_SUCCESS) {
            if (macAddress) {
                CFRelease(macAddress);
            }
            macAddress = (CFDataRef) IORegistryEntryCreateCFProperty(parentService, CFSTR("IOMACAddress"), kCFAllocatorDefault, 0);
            IOObjectRelease(parentService);
        }
        IOObjectRelease(service);
    }
    
    IOObjectRelease(iterator);
    
    return (__bridge NSData *)macAddress;
}

@end
