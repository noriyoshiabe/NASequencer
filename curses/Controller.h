#pragma once

#include "NAMidi.h"

typedef struct _Controller Controller;

extern Controller *ControllerCreate(NAMidi *namidi);
extern void ControllerDestroy(Controller *controller);
extern void ControllerProcessMessage(Controller *self);
