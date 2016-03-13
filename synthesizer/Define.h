#pragma once

#include <math.h>
#include <sys/param.h>

#define Clip(v, min, max) (min > v ? min : max < v ? max : v)
#define Timecent2Sec(tc) (pow(2.0, (double)tc * 0.000833333 /* tc / 1200.0 */))
#define cB2Value(cb) (pow(10.0, (double)cb * 0.005) /* cb / 10.0) / 20.0 */ )
#define cBAttn2Value(cb) (pow(10.0, (double)cb * -0.005)  /* -cb / 10.0) / 20.0 */)
#define Value2cB(v) (0.0000000630957 > v ? -1440.0 : (200.0 * log10(v)))
#define AbsCent2Hz(c) (8.176 * pow(2.0, (double)c * 0.000833333 /* c / 1200.0 */))
#define Cent2FreqRatio(c) (pow(2.0, (double)c * 0.000833333 /* c / 1200.0 */))
#define ConcavePositiveUnipolar(x) (1.0 - sqrt(1.0 - pow((double)x, 2.0)))
#define ConcaveNegativeUnipolar(x) (1.0 - sqrt(1.0 - pow((double)x - 1.0, 2.0)))
#define ConcavePositiveBipolar(x) ((0.0 <= x ? 1.0 : -1.0) * ConcavePositiveUnipolar(x))
#define ConcaveNegativeBipolar(x) ((0.0 <= x ? -1.0 : 1.0) * ConcavePositiveBipolar(x))
#define ConvexPositiveUnipolar(x) (sqrt(1.0 - pow((double)x - 1.0, 2.0)))
#define ConvexNegativeUnipolar(x) (sqrt(1.0 - pow((double)x, 2.0)))
#define ConvexPositiveBipolar(x) ((0.0 <= x ? ConvexPositiveUnipolar(x) : -sqrt(1.0 - pow((double)x + 1.0, 2.0))))
#define ConvexNegativeBipolar(x) ((0.0 <= x ? -1.0 : 1.0) * ConvexPositiveBipolar(x))
#define SwitchPositiveUnipolar(x) (0.5 > x ? 0.0 : 1.0)
#define SwitchNegativeUnipolar(x) (0.5 > x ? 1.0 : 0.0)
#define SwitchPositiveBipolar(x) (0.0 > x ? -1.0 : 1.0)
#define SwitchNegativeBipolar(x) (0.0 > x ? 1.0 : -1.0)
