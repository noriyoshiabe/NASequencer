#pragma once

#include "Mixer.h"
#include "Controller.h"

typedef struct _ChannelView ChannelView;

extern ChannelView *ChannelViewCreate(Mixer *mixer, int channel);
extern void ChannelViewSetController(ChannelView *self, Controller *controller);
