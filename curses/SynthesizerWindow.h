#pragma once

#include "Controller.h"

typedef struct _SynthesizerWindow SynthesizerWindow;

extern SynthesizerWindow *SynthesizerWindowCreate(int channel);
extern void SynthesizerWindowDestroy(SynthesizerWindow *self);
extern int SynthesizerWindowGetChannel(SynthesizerWindow *self);
extern void SynthesizerWindowSetController(SynthesizerWindow *self, Controller *controller);
