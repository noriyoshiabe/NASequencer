#include "Define.h"

#include <stdio.h>
int main(int argc, char **argv)
{
    printf("ConcavePositiveUnipolar\n");
    for (int i = 0; i <= 10; ++i) {
        printf("-- %0.8f\n", ConcavePositiveUnipolar(0.1 * i));
    }

    printf("ConcaveNegativeUnipolar\n");
    for (int i = 0; i <= 10; ++i) {
        printf("-- %0.8f\n", ConcaveNegativeUnipolar(0.1 * i));
    }

    printf("ConcavePositiveBipolar\n");
    for (int i = 0; i <= 20; ++i) {
        printf("-- %0.8f\n", ConcavePositiveBipolar(-1.0 + 0.1 * i));
    }

    printf("ConcaveNegativeUnipolar\n");
    for (int i = 0; i <= 20; ++i) {
        printf("-- %0.8f\n", ConcaveNegativeBipolar(-1.0 + 0.1 * i));
    }

    printf("ConvexPositiveUnipolar\n");
    for (int i = 0; i <= 10; ++i) {
        printf("-- %0.8f\n", ConvexPositiveUnipolar(0.1 * i));
    }

    printf("ConvexNegativeUnipolar\n");
    for (int i = 0; i <= 10; ++i) {
        printf("-- %0.8f\n", ConvexNegativeUnipolar(0.1 * i));
    }

    printf("ConvexPositiveBipolar\n");
    for (int i = 0; i <= 20; ++i) {
        printf("-- %0.8f\n", ConvexPositiveBipolar(-1.0 + 0.1 * i));
    }

    printf("ConvexNegativeUnipolar\n");
    for (int i = 0; i <= 20; ++i) {
        printf("-- %0.8f\n", ConvexNegativeBipolar(-1.0 + 0.1 * i));
    }

    printf("SwitchPositiveUnipolar\n");
    for (int i = 0; i <= 10; ++i) {
        printf("-- %0.8f\n", SwitchPositiveUnipolar(0.1 * i));
    }

    printf("SwitchNegativeUnipolar\n");
    for (int i = 0; i <= 10; ++i) {
        printf("-- %0.8f\n", SwitchNegativeUnipolar(0.1 * i));
    }

    printf("SwitchPositiveBipolar\n");
    for (int i = 0; i <= 20; ++i) {
        printf("-- %0.8f\n", SwitchPositiveBipolar(-1.0 + 0.1 * i));
    }

    printf("SwitchNegativeUnipolar\n");
    for (int i = 0; i <= 20; ++i) {
        printf("-- %0.8f\n", SwitchNegativeBipolar(-1.0 + 0.1 * i));
    }

    return 0;
}
