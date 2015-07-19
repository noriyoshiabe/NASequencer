#pragma once

#include "SoundFont.h"
#include "Channel.h"
#include "Voice.h"

typedef SFModList Modulator;

extern int16_t ModulatorGetValue(const Modulator *self, Channel *channel, Voice *voice);
extern bool ModulatorIsIdentical(const Modulator *self, const Modulator *to);
extern void ModulatorAddOverwrite(const Modulator **modList, int *count, const Modulator *toAdd);
