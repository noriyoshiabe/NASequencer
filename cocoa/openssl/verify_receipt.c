#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/pkcs7.h>
#include <openssl/err.h>

#include "NAArray.h"
#include "NAMap.h"
#include "NACInteger.h"
#include "NAByteBuffer.h"

typedef enum {
    ReceiptKeyBundleID,
    ReceiptKeyBundleIDHash,
    ReceiptKeyBundleShortVersion,
    ReceiptKeyOriginalAppVersion,
    ReceiptKeyExpirationDate,
    ReceiptKeyOpaque,
    ReceiptKeySha1Hash,
    ReceiptKeyInAppPurchase,
    ReceiptKeyInAppPurchaseQuantity,
    ReceiptKeyInAppPurchaseProductId,
    ReceiptKeyInAppPurchaseTransactionId,
    ReceiptKeyInAppPurchasePurchaseDate,
    ReceiptKeyInAppPurchaseOriginalTransactionId,
    ReceiptKeyInAppPurchaseOriginalPurchaseDate,
    ReceiptKeyInAppPurchaseSubscriptionExpirationDate,
    ReceiptKeyInAppPurchaseWebOrderLineItemID,
    ReceiptKeyInAppPurchaseCancelDate,
} ReceiptKey;

static NAArray *parseReceipt(const uint8_t *p, const uint8_t *end);
static NAArray *parseASNSet(const uint8_t *p, const uint8_t *end);
static NAMap *parseASNSequence(const uint8_t *p, const uint8_t *end);
static char *parseASNString(const uint8_t *p, const uint8_t *end);
static int *parseASNInteger(const uint8_t *p, const uint8_t *end);
static NAByteBuffer *buildASNBytes(const uint8_t *p, size_t length);

static inline uint8_t *readBinary(const char *filepath, size_t *length)
{
    uint8_t *ret = NULL;
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        exit(173);
    }

    fseek(fp, 0L, SEEK_END);
    *length = ftell(fp); 
    rewind(fp);
    ret = malloc(*length);
    fread(ret, *length, 1, fp);
    fclose(fp);

    return ret;
}

static NAArray *parseReceipt(const uint8_t *p, const uint8_t *end)
{
    int type = 0;
	int xclass = 0;
	long length = 0;

    ASN1_get_object(&p, &length, &type, &xclass, end - p);

    if (V_ASN1_SET == type) {
        return parseASNSet(p, end);
    }
    else {
        return NULL;
    }
}

static NAArray *parseASNSet(const uint8_t *p, const uint8_t *end)
{
    int type = 0;
    int xclass = 0;
    long length = 0;

    NAArray *array =  NAArrayCreate(4, NADescriptionAddress);

    while (p < end) {
        ASN1_get_object(&p, &length, &type, &xclass, end - p);
        NAArrayAppend(array, parseASNSequence(p, p + length));
        p += length;
    }

    return array;
}

static NAMap *parseASNSequence(const uint8_t *p, const uint8_t *end)
{
    NAMap *map = NAMapCreate(NAHashCInteger, NADescriptionCInteger, NADescriptionAddress);

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

        char *string = NULL;

        ASN1_get_object(&p, &length, &type, &xclass, end - p);
        switch (attr_type) {
        case 2: // CFBundleIdentifier
            NAMapPut(map, NACIntegerFromInteger(ReceiptKeyBundleIDHash), buildASNBytes(p, length));
            NAMapPut(map, NACIntegerFromInteger(ReceiptKeyBundleID), parseASNString(p, end));
            break;
        case 3: // CFBundleShortVersionString
            NAMapPut(map, NACIntegerFromInteger(ReceiptKeyBundleShortVersion), parseASNString(p, end));
            break;
        case 19: // Origianl App Version
            NAMapPut(map, NACIntegerFromInteger(ReceiptKeyOriginalAppVersion), parseASNString(p, end));
            break;
        case 21: // Expiration Date
            NAMapPut(map, NACIntegerFromInteger(ReceiptKeyExpirationDate), parseASNString(p, end));
            break;
        case 4: // Opaque
            NAMapPut(map, NACIntegerFromInteger(ReceiptKeyOpaque), buildASNBytes(p, length));
            break;
        case 5: // SHA-1 Hash
            NAMapPut(map, NACIntegerFromInteger(ReceiptKeySha1Hash), buildASNBytes(p, length));
            break;
        case 17: // In App purchase
            NAMapPut(map, NACIntegerFromInteger(ReceiptKeyInAppPurchase), parseReceipt(p, p + length));
            break;
        case 1701: // In App purchase - Quantity
            NAMapPut(map, NACIntegerFromInteger(ReceiptKeyInAppPurchaseQuantity), parseASNInteger(p, end));
            break;
        case 1702: // In App purchase - Product Id
            NAMapPut(map, NACIntegerFromInteger(ReceiptKeyInAppPurchaseProductId), parseASNString(p, end));
            break;
        case 1703: // In App purchase - Transaction Id
            NAMapPut(map, NACIntegerFromInteger(ReceiptKeyInAppPurchaseTransactionId), parseASNString(p, end));
            break;
        case 1704: // In App purchase - Purchase Date
            NAMapPut(map, NACIntegerFromInteger(ReceiptKeyInAppPurchasePurchaseDate), parseASNString(p, end));
            break;
        case 1705: // In App purchase - Original Transaction Id
            NAMapPut(map, NACIntegerFromInteger(ReceiptKeyInAppPurchaseOriginalTransactionId), parseASNString(p, end));
            break;
        case 1706: // In App purchase - Original Purchase Date
            NAMapPut(map, NACIntegerFromInteger(ReceiptKeyInAppPurchaseOriginalPurchaseDate), parseASNString(p, end));
            break;
        case 1708: // In App purchase - Subscription Expiration Date
            NAMapPut(map, NACIntegerFromInteger(ReceiptKeyInAppPurchaseSubscriptionExpirationDate), parseASNString(p, end));
            break;
        case 1711: // In App purchase - Web Order Line Item ID
            NAMapPut(map, NACIntegerFromInteger(ReceiptKeyInAppPurchaseWebOrderLineItemID), parseASNInteger(p, end));
            break;
        case 1712: // In App purchase - Cancel Date
            NAMapPut(map, NACIntegerFromInteger(ReceiptKeyInAppPurchaseCancelDate), parseASNString(p, end));
            break;
        }
		p += length;
    }

    return map;
}

static char *parseASNString(const uint8_t *p, const uint8_t *end)
{
    int type = 0;
	int xclass = 0;
    long length = 0;

    const uint8_t *str_p = p;
    ASN1_get_object(&str_p, &length, &type, &xclass, end - p);
    char *str = malloc(length + 1);
    strncpy(str, (const char *)str_p, length);
    str[length] = '\0';
    
    return str;
}

static int *parseASNInteger(const uint8_t *p, const uint8_t *end)
{
    int type = 0;
	int xclass = 0;
    long length = 0;

    const uint8_t *num_p = p;
    ASN1_get_object(&num_p, &length, &type, &xclass, end - p);

    int *ret = calloc(1, sizeof(int));

    if (length) {
        *ret += num_p[0];
        if (1 < length) {
            *ret += num_p[1] << 8;
            if (2 < length) {
                *ret += num_p[2] << 16;
                if (3 < length) {
                    *ret += num_p[3] << 24;
                }
            }
        }
    }

    return ret;
}


static NAByteBuffer *buildASNBytes(const uint8_t *p, size_t length)
{
    NAByteBuffer *buffer = NAByteBufferCreate(length);
    NAByteBufferWriteData(buffer, (void *)p, length);

    return buffer;
}

int main(int argc, char **argv)
{
    OpenSSL_add_all_digests();

    size_t x509Length;
    size_t receiptLength;

    uint8_t *x509Buffer = readBinary(argv[1], &x509Length);
    uint8_t *receiptBuffer = readBinary(argv[2], &receiptLength);

    BIO *b_x509 = BIO_new_mem_buf(x509Buffer, x509Length);
    BIO *b_receipt = BIO_new_mem_buf(receiptBuffer, receiptLength);

    X509_STORE *store = X509_STORE_new();
    X509 *appleCA = d2i_X509_bio(b_x509, NULL);
    X509_STORE_add_cert(store, appleCA);

    PKCS7 *p7 = d2i_PKCS7_bio(b_receipt, NULL);

    BIO *b_receiptPayload = BIO_new(BIO_s_mem());
    int result = PKCS7_verify(p7, NULL, store, NULL, b_receiptPayload, 0);
    if (1 != result) {
        ERR_print_errors_fp(stdout);
        exit(173);
    }

    BIO_free(b_receiptPayload);

    BIO_free(b_receipt);

    X509_free(appleCA);
    X509_STORE_free(store);
    BIO_free(b_x509);

    free(x509Buffer);
    free(receiptBuffer);


    ASN1_OCTET_STRING *octets = p7->d.sign->contents->d.data;

    NAArray *receipt = parseReceipt(octets->data, octets->data + octets->length);

    PKCS7_free(p7);

    return 0;
}
