#pragma once

#include "SoundFont.h"
#include "Channel.h"
#include "Voice.h"

typedef SFModList Modulator;

extern int16_t ModulatorGetValue(Modulator *self, Channel *channel, Voice *voice);
