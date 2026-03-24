#include "Interface.h"
#include "MasterView.h"
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
    FocusGain,
    FocusVolume,
} Focus;

struct _MasterView {
    const InterfaceVtbl *vtbl;
    ViewNode *node;
    KeyHandler *nextKeyHandler;
    Mixer *mixer;
    Focus focus;
    bool selected;
};

static ViewNode *MasterViewGetNode(View *self);
static void MasterViewDraw(View *self, Size size);
static void MasterViewDestroy(View *self);

static const ViewVtbl MasterViewViewVtbl = {
    .getNode = MasterViewGetNode,
    .draw = MasterViewDraw,
    .destroy = MasterViewDestroy,
};

static bool MasterViewOnKeyEvent(KeyHandler *self, int code);
static void MasterViewSetNextKeyHandler(KeyHandler *self, KeyHandler *next);
static KeyHandler *MasterViewGetNextKeyHandler(KeyHandler *self);

static const KeyHandlerVtbl MasterViewKeyHandlerVtbl = {
    .onKeyEvent = MasterViewOnKeyEvent,
    .setNextKeyHandler = MasterViewSetNextKeyHandler,
    .getNextKeyHandler = MasterViewGetNextKeyHandler,
};

static void *MasterViewQueryInteface(void *self, IID iid)
{
    if (iid == IIDView)
        return (void*)&MasterViewViewVtbl;
    if (iid == IIDKeyHandler)
        return (void*)&MasterViewKeyHandlerVtbl;
    return NULL;
}

static const InterfaceVtbl MasterViewInterfaceVtbl = {
    .queryInterface = MasterViewQueryInteface,
};

static MixerObserverCallbacks MasterViewMixerObserverCallbacks;

MasterView *MasterViewCreate(Mixer *mixer)
{
    MasterView *self = calloc(1, sizeof(MasterView));
    self->vtbl = &MasterViewInterfaceVtbl;
    self->node = ViewNodeCreate(self);

    self->mixer = mixer;

    MixerAddObserver(self->mixer, self, &MasterViewMixerObserverCallbacks);
    return self;
}

static ViewNode *MasterViewGetNode(View *_self)
{
    MasterView *self = _self;
    return self->node;
}

static void MasterViewDraw(View *_self, Size size)
{
    MasterView *self = _self;

    MidiSourceManager *sourceManager = MidiSourceManagerSharedInstance();
    MidiSourceDescription *description = MidiSourceManagerGetDefaultDescription(sourceManager);

    bool isKeyView = ViewIsKeyView(self);

    bool isGainFocused = isKeyView && self->focus == FocusGain;
    bool isVolumeFocused = isKeyView && self->focus == FocusVolume;

    ViewSetAttr(self, AttributeDefault);
    ViewPrintf(self, 0, 0, "Master");

    int left = size.width - 25;

    ViewSetAttr(self, AttributeDefault);
    ViewPrintf(self, left +  0, 0, "Gain");
    ViewSetAttr(self, Attribute(ColorDefault, isGainFocused, self->selected));
    ViewPrintf(self, left +  5, 0, "[%3d]", description->gain);

    ViewSetAttr(self, AttributeDefault);
    ViewPrintf(self, left +  11, 0, "Volume");
    ViewSetAttr(self, Attribute(ColorDefault, isVolumeFocused, self->selected));
    ViewPrintf(self, left +  18, 0, "[%3d]", description->masterVolume);

    Level level = MixerGetLevel(self->mixer);

    int levelMax = (size.width - 3);
    int peak = levelMax / 10;
    int levelL = (int)(((MAX(-500, level.L) + 500) / 500.0) * levelMax);
    int levelR = (int)(((MAX(-500, level.R) + 500) / 500.0) * levelMax);

    char bar[levelMax + 1];

    ViewSetAttr(self, AttributeDefault);
    ViewPrintf(self, 0, 1, "L: ");

    ViewSetAttr(self, Attribute(ColorLevelNone, false, true));
    memset(bar, '|', levelMax);
    bar[levelMax] = '\0';
    ViewPrintf(self, 3, 1, bar);

    ViewSetAttr(self, Attribute(ColorLevelLow, false, true));
    int levelGreenL = MIN(levelL, levelMax - peak);
    memset(bar, '|', levelGreenL);
    bar[levelGreenL] = '\0';
    ViewPrintf(self, 3, 1, bar);
    if (levelGreenL < levelL) {
        ViewSetAttr(self, Attribute(ColorLevelHigh, false, true));
        bar[levelL - levelGreenL] = '\0';
        ViewPrintf(self, size.width - peak, 1, bar);
    }

    ViewSetAttr(self, ColorDefault);
    ViewPrintf(self, 0, 2, "R: ");

    ViewSetAttr(self, Attribute(ColorLevelNone, false, true));
    memset(bar, '|', levelMax);
    bar[levelMax] = '\0';
    ViewPrintf(self, 3, 2, bar);

    ViewSetAttr(self, Attribute(ColorLevelLow, false, true));
    int levelGreenR = MIN(levelR, levelMax - peak);
    memset(bar, '|', levelGreenR);
    bar[levelGreenR] = '\0';
    ViewPrintf(self, 3, 2, bar);
    if (levelGreenR < levelR) {
        ViewSetAttr(self, Attribute(ColorLevelHigh, false, true));
        bar[levelR - levelGreenR] = '\0';
        ViewPrintf(self, size.width - peak, 2, bar);
    }
}

static bool MasterViewOnKeyEvent(KeyHandler *_self, int code)
{
    MasterView *self = _self;

    MidiSourceManager *sourceManager = MidiSourceManagerSharedInstance();
    MidiSourceDescription *description = MidiSourceManagerGetDefaultDescription(sourceManager);

    if (self->selected) {
        switch (code) {
        case 27:
        case '\n':
            self->selected = false;
            return true;
        case KEY_LEFT:
        case KEY_DOWN:
            switch (self->focus) {
            case FocusGain:
                MidiSourceManagerSetGainForDescription(sourceManager, description, MAX(-1440, description->gain - 1));
                return true;
            case FocusVolume:
                MidiSourceManagerSetMasterVolumeForDescription(sourceManager, description, MAX(-1440, description->masterVolume - 1));
                return true;
            default:
                return true;
            }
        case KEY_RIGHT:
        case KEY_UP:
            switch (self->focus) {
            case FocusGain:
                MidiSourceManagerSetGainForDescription(sourceManager, description, MIN(200, description->gain + 1));
                return true;
            case FocusVolume:
                MidiSourceManagerSetMasterVolumeForDescription(sourceManager, description, MIN(200, description->masterVolume + 1));
                return true;
            default:
                return true;
            }
        }
    } else {
        switch (code) {
        case '\n':
            self->selected = !self->selected;
            return true;
        case KEY_LEFT:
            if (FocusGain < self->focus) {
                --self->focus;
            }
            return true;
        case KEY_RIGHT:
            if (self->focus < FocusVolume) {
                ++self->focus;
            }
            return true;
        }
    }

    return false;
}

static void MasterViewSetNextKeyHandler(KeyHandler *_self, KeyHandler *next)
{
    MasterView *self = _self;
    self->nextKeyHandler = next;
}

static KeyHandler *MasterViewGetNextKeyHandler(KeyHandler *_self)
{
    MasterView *self = _self;
    return self->nextKeyHandler;
}

static void MasterViewDestroy(View *_self)
{
    MasterView *self = _self;
    MixerRemoveObserver(self->mixer, self);
    ViewNodeDestroy(self->node);
    free(self);
}

static void MasterViewMixerOnChannelStatusChange(void *receiver, MixerChannel *channel, MixerChannelStatusKind kind)
{
}

static void MasterViewMixerOnAvailableMidiSourceChange(void *receiver, NAArray *descriptions)
{
}

static void MasterViewMixerOnLevelUpdate(void *receiver)
{
    MasterView *self = receiver;
    ViewInvalidate(self);
}

static MixerObserverCallbacks MasterViewMixerObserverCallbacks = {
    MasterViewMixerOnChannelStatusChange,
    MasterViewMixerOnAvailableMidiSourceChange,
    MasterViewMixerOnLevelUpdate,
};
