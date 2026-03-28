#include "PresetHelper.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

PresetInfo *PresetHelperGetNextPresetInfo(MixerChannel *mixerChannel)
{
    int count = MixerChannelGetPresetCount(mixerChannel);
    PresetInfo **presetInfos = MixerChannelGetPresetInfos(mixerChannel);
    PresetInfo *presetInfo = MixerChannelGetPresetInfo(mixerChannel);
    
    int index = 0;
    for (int i = 0; i < count; ++i) {
        if (presetInfo == presetInfos[i]) {
            index = MIN(count - 1, i + 1);
            break;
        }
    }

    return presetInfos[index];
}

PresetInfo *PresetHelperGetPreviousPresetInfo(MixerChannel *mixerChannel)
{
    int count = MixerChannelGetPresetCount(mixerChannel);
    PresetInfo **presetInfos = MixerChannelGetPresetInfos(mixerChannel);
    PresetInfo *presetInfo = MixerChannelGetPresetInfo(mixerChannel);
    
    int index = 0;
    for (int i = 0; i < count; ++i) {
        if (presetInfo == presetInfos[i]) {
            index = MAX(0, i - 1);
            break;
        }
    }

    return presetInfos[index];
}

int PresetHelperFindPresetInfoIndex(MixerChannel *mixerChannel, PresetInfo *presetInfo)
{
    int count = MixerChannelGetPresetCount(mixerChannel);
    PresetInfo **presetInfos = MixerChannelGetPresetInfos(mixerChannel);

    for (int i = 0; i < count; ++i) {
        if (presetInfo == presetInfos[i]) {
            return i;
        }
    }

    return -1;
}
