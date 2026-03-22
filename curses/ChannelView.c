#include "Interface.h"
#include "View.h"
#include "ViewNode.h"
#include "ChannelView.h"
#include "Color.h"

#include <string.h>
#include <stdlib.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

struct _ChannelView {
    const InterfaceVtbl *vtbl;
    ViewNode *node;
    Mixer *mixer;
    MixerChannel *mixerChannel;
    int channel;
};

static ViewNode *ChannelViewGetNode(View *self);
static void ChannelViewDraw(View *self, Size size);
static void ChannelViewDestroy(View *self);

static const ViewVtbl ChannelViewViewVtbl = {
    .getNode = ChannelViewGetNode,
    .draw = ChannelViewDraw,
    .destroy = ChannelViewDestroy,
};

static void *ChannelViewQueryInteface(void *self, IID iid)
{
    if (iid == IIDView)
        return (void*)&ChannelViewViewVtbl;
    return NULL;
}

static const InterfaceVtbl ChannelViewInterfaceVtbl = {
    .queryInterface = ChannelViewQueryInteface,
};

static MixerObserverCallbacks ChannelViewMixerObserverCallbacks;

ChannelView *ChannelViewCreate(Mixer *mixer, int channel)
{
    ChannelView *self = calloc(1, sizeof(ChannelView));
    self->vtbl = &ChannelViewInterfaceVtbl;
    self->node = ViewNodeCreate(self);

    self->mixer = mixer;
    self->mixerChannel = NAArrayGetValueAt(MixerGetChannels(mixer), channel - 1);
    self->channel = channel;

    MixerAddObserver(self->mixer, self, &ChannelViewMixerObserverCallbacks);
    return self;
}

static ViewNode *ChannelViewGetNode(View *_self)
{
    ChannelView *self = (ChannelView *)_self;
    return self->node;
}

static void ChannelViewDraw(View *_self, Size size)
{
    ChannelView *self = (ChannelView *)_self;

    ViewPrintf(self, 0, 0, "Channel: %2d", self->channel);

    bool mute = MixerChannelGetMute(self->mixerChannel);
    bool solo = MixerChannelGetSolo(self->mixerChannel);
    int volume = MixerChannelGetVolume(self->mixerChannel);
    int pan = MixerChannelGetPan(self->mixerChannel);
    int chorus = MixerChannelGetChorusSend(self->mixerChannel);
    int reverb = MixerChannelGetReverbSend(self->mixerChannel);

    MidiSourceDescription *description = MixerChannelGetMidiSourceDescription(self->mixerChannel);
    PresetInfo *preset = MixerChannelGetPresetInfo(self->mixerChannel);

    ViewSetColor(self, mute ? ColorMute : ColorDefault);
    ViewPrintf(self, 12, 0, "[Mute]");
    ViewSetColor(self, solo ? ColorSolo : ColorDefault);
    ViewPrintf(self, 19, 0, "[Solo]");

    int left = size.width - 47;
    ViewSetColor(self, ColorDefault);

    ViewPrintf(self, left +  0, 0, "Volume");
    ViewPrintf(self, left +  7, 0, "[%3d]", volume);
    ViewPrintf(self, left + 13, 0, "Pan");
    ViewPrintf(self, left + 17, 0, pan == 0 ? "[%3d]" : "[%+3d]", pan);
    ViewPrintf(self, left + 23, 0, "Chorus");
    ViewPrintf(self, left + 30, 0, "[%3d]", chorus);
    ViewPrintf(self, left + 36, 0, "Reverb");
    ViewPrintf(self, left + 42, 0, "[%3d]", reverb);

    ViewPrintf(self, 0, 1, "Syntesizer");
    ViewPrintf(self, 11, 1, "[%s]", description->name);

    left = 14 + strlen(description->name);
    ViewPrintf(self, left, 1, "Preset");
    ViewPrintf(self, left + 7, 1, "[%s]", preset->name);

    Level level = MixerChannelGetLevel(self->mixerChannel);

    int levelMax = (size.width - 3);
    int peak = levelMax / 10;
    int levelL = (int)(((MAX(-500, level.L) + 500) / 500.0) * levelMax);
    int levelR = (int)(((MAX(-500, level.R) + 500) / 500.0) * levelMax);

    char bar[levelMax + 1];

    ViewSetColor(self, ColorDefault);
    ViewPrintf(self, 0, 2, "L: ");

    ViewSetColor(self, ColorLevelLow);
    int levelGreenL = MIN(levelL, levelMax - peak);
    memset(bar, '|', levelGreenL);
    bar[levelGreenL] = '\0';
    ViewPrintf(self, 3, 2, bar);
    if (levelGreenL < levelL) {
        ViewSetColor(self, ColorLevelHigh);
        bar[levelL - levelGreenL] = '\0';
        ViewPrintf(self, size.width - peak, 2, bar);
    }

    ViewSetColor(self, ColorDefault);
    ViewPrintf(self, 0, 3, "R: ");

    ViewSetColor(self, ColorLevelLow);
    int levelGreenR = MIN(levelR, levelMax - peak);
    memset(bar, '|', levelGreenR);
    bar[levelGreenR] = '\0';
    ViewPrintf(self, 3, 3, bar);
    if (levelGreenR < levelR) {
        ViewSetColor(self, ColorLevelHigh);
        bar[levelR - levelGreenR] = '\0';
        ViewPrintf(self, size.width - peak, 3, bar);
    }

    ViewSetColor(self, ColorDefault);
    char line[size.width + 1];
    memset(line, '-', size.width);
    line[size.width] = '\0';
    ViewPrintf(self, 0, 4, line);
}

static void ChannelViewDestroy(View *_self)
{
    ChannelView *self = (ChannelView *)_self;
    MixerRemoveObserver(self->mixer, self);
    ViewNodeDestroy(self->node);
    free(self);
}

static void ChannelViewMixerOnChannelStatusChange(void *receiver, MixerChannel *channel, MixerChannelStatusKind kind)
{
    ChannelView *self = receiver;
    ViewInvalidate(self);
}

static void ChannelViewMixerOnAvailableMidiSourceChange(void *receiver, NAArray *descriptions)
{
}

static void ChannelViewMixerOnLevelUpdate(void *receiver)
{
    ChannelView *self = receiver;
    ViewInvalidate(self);
}

static MixerObserverCallbacks ChannelViewMixerObserverCallbacks = {
    ChannelViewMixerOnChannelStatusChange,
    ChannelViewMixerOnAvailableMidiSourceChange,
    ChannelViewMixerOnLevelUpdate,
};
