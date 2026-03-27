#pragma once

#include "NAMidi.h"

typedef struct _Controller Controller;

extern Controller *ControllerCreate(NAMidi *namidi);
extern void ControllerDestroy(Controller *controller);
extern void ControllerPostMessage(Controller *self, int kind, void *data);
extern void ControllerProcessMessage(Controller *self);

typedef enum {
    ControllerMessageParseErrors,
    ControllerMessageOpenSynthesizerWindow,
    ControllerMessageCloseSynthesizerWindow,
} ControllerMessage;
