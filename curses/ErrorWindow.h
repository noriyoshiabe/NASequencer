#pragma once

#include "NAArray.h"

typedef struct _ErrorWindow ErrorWindow;

extern ErrorWindow *ErrorWindowCreate();
extern void ErrorWindowDestroy(ErrorWindow *self);
extern void ErrorWindowDraw(ErrorWindow *self, NAArray *errors);
