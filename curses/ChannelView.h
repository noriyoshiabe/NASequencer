#pragma once

#include "Mixer.h"

typedef struct _ChannelView ChannelView;

extern ChannelView *ChannelViewCreate(Mixer *mixer, int channel);
