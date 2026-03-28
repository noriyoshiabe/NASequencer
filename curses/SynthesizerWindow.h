#pragma once

#include "Mixer.h"
#include "Controller.h"

typedef struct _SynthesizerWindow SynthesizerWindow;

extern SynthesizerWindow *SynthesizerWindowCreate(Mixer *mixer, int channel);
extern void SynthesizerWindowDestroy(SynthesizerWindow *self);
extern int SynthesizerWindowGetChannel(SynthesizerWindow *self);
extern void SynthesizerWindowSetController(SynthesizerWindow *self, Controller *controller);
