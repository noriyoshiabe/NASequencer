#pragma once

#include "View.h"
#include "NAMidi.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

extern void ViewHelperDisplayLevel(View *view, int yOffset, Level level);
