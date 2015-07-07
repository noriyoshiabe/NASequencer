#pragma once

#include <math.h>
#include <sys/param.h>

#define Clip(v, min, max) (MIN(MAX(v, min), max))
#define Timecent2Sec(tc) (pow(2.0, (double)tc / 1200.0))
#define cBAttn2Value(cb) (pow(10.0, ((double)-cb / 10.0) / 20.0))
#define AbsCent2Hz(c) (8.176 * pow(2.0, (double)c / 1200.0))
#define ConvexPositiveUnipolar(x) (sqrt(1.0 - pow((double)x - 1.0, 2.0)))
