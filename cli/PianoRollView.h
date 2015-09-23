#pragma once

#include "NAMidi.h"

typedef struct _PianoRollView PianoRollView;

extern PianoRollView *PianoRollViewCreate(NAMidi *namidi);
extern void PianoRollViewDestroy(PianoRollView *self);
extern void PianoRollViewSetFrom(PianoRollView *self, int from);
extern void PianoRollViewSetLength(PianoRollView *self, int length);
extern void PianoRollViewRender(PianoRollView *self);
extern void PianoRollViewSetSequence(PianoRollView *self, Sequence *sequence);
