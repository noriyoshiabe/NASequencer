#pragma once

#include "Mixer.h"

extern PresetInfo *PresetHelperGetNextPresetInfo(MixerChannel *mixerChannel);
extern PresetInfo *PresetHelperGetPreviousPresetInfo(MixerChannel *mixerChannel);
extern int PresetHelperFindPresetInfoIndex(MixerChannel *mixerChannel, PresetInfo *presetInfo);
