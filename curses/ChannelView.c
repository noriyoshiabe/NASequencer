#include "Interface.h"
#include "ChannelView.h"
#include "View.h"
#include "ViewNode.h"
#include "KeyHandler.h"
#include "Attribute.h"
#include "Debug.h"

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef enum {
    FocusMute,
    FocusSolo,
    FocusVolume,
    FocusPan,
    FocusCholus,
    FocusReverb,
    FocusSyntesizer,
    FocusPreset,
} Focus;

struct _ChannelView {
    const InterfaceVtbl *vtbl;
    ViewNode *node;
    KeyHandler *nextKeyHandler;
    Mixer *mixer;
    MixerChannel *mixerChannel;
    int channel;
    Focus focus;
    bool selected;
};

static ViewNode *ChannelViewGetNode(View *self);
static void ChannelViewDraw(View *self, Size size);
static void ChannelViewDestroy(View *self);

static const ViewVtbl ChannelViewViewVtbl = {
    .getNode = ChannelViewGetNode,
    .draw = ChannelViewDraw,
    .destroy = ChannelViewDestroy,
};

static bool ChannelViewOnKeyEvent(KeyHandler *self, int code);
static void ChannelViewSetNextKeyHandler(KeyHandler *self, KeyHandler *next);
static KeyHandler *ChannelViewGetNextKeyHandler(KeyHandler *self);

static const KeyHandlerVtbl ChannelViewKeyHandlerVtbl = {
    .onKeyEvent = ChannelViewOnKeyEvent,
    .setNextKeyHandler = ChannelViewSetNextKeyHandler,
    .getNextKeyHandler = ChannelViewGetNextKeyHandler,
};

static void *ChannelViewQueryInteface(void *self, IID iid)
{
    if (iid == IIDView)
        return (void*)&ChannelViewViewVtbl;
    if (iid == IIDKeyHandler)
        return (void*)&ChannelViewKeyHandlerVtbl;
    return NULL;
}

static const InterfaceVtbl ChannelViewInterfaceVtbl = {
    .queryInterface = ChannelViewQueryInteface,
};

static MixerObserverCallbacks ChannelViewMixerObserverCallbacks;

static PresetInfo *getNextPresetInfo(MixerChannel *mixerChannel);
static PresetInfo *getPreviousPresetInfo(MixerChannel *mixerChannel);

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
    ChannelView *self = _self;

    bool isKeyView = ViewIsKeyView(self);

    bool isMuteFocused = isKeyView && self->focus == FocusMute;
    bool isSoloFocused = isKeyView && self->focus == FocusSolo;
    bool isVolumeFocused = isKeyView && self->focus == FocusVolume;
    bool isPanFocused = isKeyView && self->focus == FocusPan;
    bool isCholusFocused = isKeyView && self->focus == FocusCholus;
    bool isReverbFocused = isKeyView && self->focus == FocusReverb;
    bool isSyntesizerFocused = isKeyView && self->focus == FocusSyntesizer;
    bool isPresetFocused = isKeyView && self->focus == FocusPreset;

    ViewPrintf(self, 0, 0, "Channel: %2d", self->channel);

    bool mute = MixerChannelGetMute(self->mixerChannel);
    bool solo = MixerChannelGetSolo(self->mixerChannel);
    int volume = MixerChannelGetVolume(self->mixerChannel);
    int pan = MixerChannelGetPan(self->mixerChannel);
    int chorus = MixerChannelGetChorusSend(self->mixerChannel);
    int reverb = MixerChannelGetReverbSend(self->mixerChannel);

    MidiSourceDescription *description = MixerChannelGetMidiSourceDescription(self->mixerChannel);
    PresetInfo *preset = MixerChannelGetPresetInfo(self->mixerChannel);

    ViewSetAttr(self, Attribute(mute ? ColorMute : ColorDefault, isMuteFocused, false));
    ViewPrintf(self, 12, 0, "[Mute]");
    ViewSetAttr(self, Attribute(solo ? ColorSolo : ColorDefault, isSoloFocused, false));
    ViewPrintf(self, 19, 0, "[Solo]");

    int left = size.width - 47;

    ViewSetAttr(self, AttributeDefault);
    ViewPrintf(self, left +  0, 0, "Volume");
    ViewSetAttr(self, Attribute(ColorDefault, isVolumeFocused, self->selected));
    ViewPrintf(self, left +  7, 0, "[%3d]", volume);

    ViewSetAttr(self, AttributeDefault);
    ViewPrintf(self, left + 13, 0, "Pan");
    ViewSetAttr(self, Attribute(ColorDefault, isPanFocused, self->selected));
    ViewPrintf(self, left + 17, 0, pan == 0 ? "[%3d]" : "[%+3d]", pan);

    ViewSetAttr(self, AttributeDefault);
    ViewPrintf(self, left + 23, 0, "Chorus");
    ViewSetAttr(self, Attribute(ColorDefault, isCholusFocused, self->selected));
    ViewPrintf(self, left + 30, 0, "[%3d]", chorus);

    ViewSetAttr(self, AttributeDefault);
    ViewPrintf(self, left + 36, 0, "Reverb");
    ViewSetAttr(self, Attribute(ColorDefault, isReverbFocused, self->selected));
    ViewPrintf(self, left + 42, 0, "[%3d]", reverb);

    ViewSetAttr(self, AttributeDefault);
    ViewPrintf(self, 0, 1, "Syntesizer");
    ViewSetAttr(self, Attribute(ColorDefault, isSyntesizerFocused, self->selected));
    ViewPrintf(self, 11, 1, "[%s]", description->name);

    left = 14 + strlen(description->name);

    ViewSetAttr(self, AttributeDefault);
    ViewPrintf(self, left, 1, "Preset");
    ViewSetAttr(self, Attribute(ColorDefault, isPresetFocused, self->selected));
    ViewPrintf(self, left + 7, 1, "[%s]", preset->name);

    Level level = MixerChannelGetLevel(self->mixerChannel);

    int levelMax = (size.width - 3);
    int peak = levelMax / 10;
    int levelL = (int)(((MAX(-500, level.L) + 500) / 500.0) * levelMax);
    int levelR = (int)(((MAX(-500, level.R) + 500) / 500.0) * levelMax);

    char bar[levelMax + 1];

    ViewSetAttr(self, AttributeDefault);
    ViewPrintf(self, 0, 2, "L: ");

    ViewSetAttr(self, Attribute(ColorLevelLow, false, true));
    int levelGreenL = MIN(levelL, levelMax - peak);
    memset(bar, '|', levelGreenL);
    bar[levelGreenL] = '\0';
    ViewPrintf(self, 3, 2, bar);
    if (levelGreenL < levelL) {
        ViewSetAttr(self, Attribute(ColorLevelHigh, false, true));
        bar[levelL - levelGreenL] = '\0';
        ViewPrintf(self, size.width - peak, 2, bar);
    }

    ViewSetAttr(self, ColorDefault);
    ViewPrintf(self, 0, 3, "R: ");

    ViewSetAttr(self, Attribute(ColorLevelLow, false, true));
    int levelGreenR = MIN(levelR, levelMax - peak);
    memset(bar, '|', levelGreenR);
    bar[levelGreenR] = '\0';
    ViewPrintf(self, 3, 3, bar);
    if (levelGreenR < levelR) {
        ViewSetAttr(self, Attribute(ColorLevelHigh, false, true));
        bar[levelR - levelGreenR] = '\0';
        ViewPrintf(self, size.width - peak, 3, bar);
    }

    ViewSetAttr(self, AttributeDefault);
    char line[size.width + 1];
    memset(line, '-', size.width);
    line[size.width] = '\0';
    ViewPrintf(self, 0, 4, line);
}

static bool ChannelViewOnKeyEvent(KeyHandler *_self, int code)
{
    ChannelView *self = _self;

    if (self->selected) {
        switch (code) {
        case 27:
        case '\n':
            self->selected = false;
            return true;
        case KEY_LEFT:
        case KEY_DOWN:
            switch (self->focus) {
            case FocusVolume:
                MixerChannelSetVolume(self->mixerChannel, MAX(0, MixerChannelGetVolume(self->mixerChannel) - 1));
                return true;
            case FocusPan:
                MixerChannelSetPan(self->mixerChannel, MAX(-64, MixerChannelGetPan(self->mixerChannel) - 1));
                return true;
            case FocusCholus:
                MixerChannelSetChorusSend(self->mixerChannel, MAX(0, MixerChannelGetChorusSend(self->mixerChannel) - 1));
                return true;
            case FocusReverb:
                MixerChannelSetReverbSend(self->mixerChannel, MAX(0, MixerChannelGetReverbSend(self->mixerChannel) - 1));
                return true;
            case FocusPreset:
                if (code == KEY_DOWN) {
                    MixerChannelSetPresetInfo(self->mixerChannel, getNextPresetInfo(self->mixerChannel));
                }
                return true;
            default:
                return true;
            }
        case KEY_RIGHT:
        case KEY_UP:
            switch (self->focus) {
            case FocusVolume:
                MixerChannelSetVolume(self->mixerChannel, MIN(127, MixerChannelGetVolume(self->mixerChannel) + 1));
                return true;
            case FocusPan:
                MixerChannelSetPan(self->mixerChannel, MIN(63, MixerChannelGetPan(self->mixerChannel) + 1));
                return true;
            case FocusCholus:
                MixerChannelSetChorusSend(self->mixerChannel, MIN(127, MixerChannelGetChorusSend(self->mixerChannel) + 1));
                return true;
            case FocusReverb:
                MixerChannelSetReverbSend(self->mixerChannel, MIN(127, MixerChannelGetReverbSend(self->mixerChannel) + 1));
                return true;
            case FocusPreset:
                if (code == KEY_UP) {
                    MixerChannelSetPresetInfo(self->mixerChannel, getPreviousPresetInfo(self->mixerChannel));
                }
                return true;
            default:
                return true;
            }
        }
    } else {
        switch (code) {
        case '\n':
            switch (self->focus) {
            case FocusMute:
                MixerChannelSetMute(self->mixerChannel, !MixerChannelGetMute(self->mixerChannel));
                return true;
            case FocusSolo:
                MixerChannelSetSolo(self->mixerChannel, !MixerChannelGetSolo(self->mixerChannel));
                return true;
            case FocusSyntesizer:
                // TODO
                return true;
            default:
                self->selected = !self->selected;
                return true;
            }
        case KEY_UP:
            if (self->focus <= FocusReverb) {
                return false;
            }
            self->focus = FocusMute;
            return true;
        case KEY_DOWN:
            if (FocusSyntesizer <= self->focus) {
                return false;
            }
            self->focus = FocusSyntesizer;
            return true;
        case KEY_LEFT:
            if (self->focus <= FocusReverb) {
                if (FocusMute < self->focus) {
                    --self->focus;
                }
            } else {
                if (FocusSyntesizer < self->focus) {
                    --self->focus;
                }
            }
            return true;
        case KEY_RIGHT:
            if (self->focus <= FocusReverb) {
                if (self->focus < FocusReverb) {
                    ++self->focus;
                }
            } else {
                if (self->focus < FocusPreset) {
                    ++self->focus;
                }
            }
            return true;
        }
    }

    return false;
}

static PresetInfo *getNextPresetInfo(MixerChannel *mixerChannel)
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

static PresetInfo *getPreviousPresetInfo(MixerChannel *mixerChannel)
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

static void ChannelViewSetNextKeyHandler(KeyHandler *_self, KeyHandler *next)
{
    ChannelView *self = _self;
    self->nextKeyHandler = next;
}

static KeyHandler *ChannelViewGetNextKeyHandler(KeyHandler *_self)
{
    ChannelView *self = _self;
    return self->nextKeyHandler;
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
