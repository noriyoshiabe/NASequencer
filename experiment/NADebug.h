#pragma once

#ifdef DEBUG
#include <stdio.h>
#define __Trace printf("---- %s:%s - %d\n", __FILE__, __func__, __LINE__)
#else
#define __Trace
#endif
