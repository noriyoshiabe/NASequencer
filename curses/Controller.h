#pragma once

#include "NAMidi.h"

typedef struct _Controller Controller;

extern Controller *ControllerCreate(NAMidi *namidi);
extern void ControllerDestroy(Controller *controller);
extern void ControllerRunOnLoop(Controller *self);
extern void ControllerPostMessage(Controller *self, int kind, void *data);
extern void ControllerRegisterTimer(Controller *self, void *receiver, void (*callback)(void *receiver, int64_t msec));
extern void ControllerUnregisterTimer(Controller *self, void *receiver);

typedef enum {
    ControllerMessageParseErrors,
    ControllerMessageOpenSynthesizerWindow,
    ControllerMessageCloseSynthesizerWindow,
} ControllerMessage;
