#pragma once

#include "Mixer.h"
#include "Controller.h"

typedef struct _SynthesizerView SynthesizerView;

extern SynthesizerView *SynthesizerViewCreate(Mixer *mixer, int channel);
extern void SynthesizerViewSetController(SynthesizerView *self, Controller *controller);
