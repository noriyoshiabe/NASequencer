#include "SynthesizerView.h"
#include "Interface.h"
#include "View.h"
#include "ViewNode.h"
#include "KeyHandler.h"
#include "Attribute.h"
#include "Debug.h"

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

struct _SynthesizerView {
    const InterfaceVtbl *vtbl;
    ViewNode *node;
    KeyHandler *nextKeyHandler;
    Mixer *mixer;
    MixerChannel *mixerChannel;
    int channel;
    int octave;
    Controller *controller;
};

static ViewNode *SynthesizerViewGetNode(View *self);
static void SynthesizerViewDraw(View *self, Size size);
static void SynthesizerViewDestroy(View *self);

static const ViewVtbl SynthesizerViewViewVtbl = {
    .getNode = SynthesizerViewGetNode,
    .draw = SynthesizerViewDraw,
    .destroy = SynthesizerViewDestroy,
};

static bool SynthesizerViewOnKeyEvent(KeyHandler *self, int code);
static void SynthesizerViewSetNextKeyHandler(KeyHandler *self, KeyHandler *next);
static KeyHandler *SynthesizerViewGetNextKeyHandler(KeyHandler *self);

static const KeyHandlerVtbl SynthesizerViewKeyHandlerVtbl = {
    .onKeyEvent = SynthesizerViewOnKeyEvent,
    .setNextKeyHandler = SynthesizerViewSetNextKeyHandler,
    .getNextKeyHandler = SynthesizerViewGetNextKeyHandler,
};

static void *SynthesizerViewQueryInteface(void *self, IID iid)
{
    if (iid == IIDView)
        return (void*)&SynthesizerViewViewVtbl;
    if (iid == IIDKeyHandler)
        return (void*)&SynthesizerViewKeyHandlerVtbl;
    return NULL;
}

static const InterfaceVtbl SynthesizerViewInterfaceVtbl = {
    .queryInterface = SynthesizerViewQueryInteface,
};

static void SynthesizerViewTimerCallback(void *receiver, int64_t msec);

static MixerObserverCallbacks SynthesizerViewMixerObserverCallbacks;

SynthesizerView *SynthesizerViewCreate(Mixer *mixer, int channel)
{
    SynthesizerView *self = calloc(1, sizeof(SynthesizerView));
    self->vtbl = &SynthesizerViewInterfaceVtbl;
    self->node = ViewNodeCreate(self);

    self->mixer = mixer;
    self->mixerChannel = NAArrayGetValueAt(MixerGetChannels(mixer), channel - 1);
    self->channel = channel;

    MixerAddObserver(self->mixer, self, &SynthesizerViewMixerObserverCallbacks);
    return self;
}

void SynthesizerViewSetController(SynthesizerView *self, Controller *controller)
{
    self->controller = controller;
    ControllerRegisterTimer(self->controller, self, SynthesizerViewTimerCallback);
}

static ViewNode *SynthesizerViewGetNode(View *_self)
{
    SynthesizerView *self = _self;
    return self->node;
}

static void SynthesizerViewDraw(View *_self, Size size)
{
    SynthesizerView *self = _self;

    MidiSourceDescription *description = MixerChannelGetMidiSourceDescription(self->mixerChannel);
    PresetInfo *preset = MixerChannelGetPresetInfo(self->mixerChannel);

    ViewSetAttr(self, AttributeDefault);
    ViewPrintf(self, 0, 0, "Channel: %-2d", self->channel);
    ViewPrintf(self, 0, 1, "Synthesizer Name: %s", description->name);
    ViewPrintf(self, 0, 2, "Bank No: %-3d Program No: %-3d", preset->bankNo, preset->programNo);

    ViewPrintf(self, 43, 2, "Preset Name:");
    ViewPrintf(self, 43, 3, "+---------------------");
    for (int i = 4; i <= 11; ++i) {
        ViewPrintf(self, 43, i, "|");
    }

    ViewPrintf(self, 12, 8, "*");
    ViewPrintf(self, 0, 9, "< >: Octave Shift: *C");
    ViewPrintf(self, 21, 9, self->octave == 0 ? "%-3d" : "%+-2d", self->octave);

    ViewPrintf(self, 0, 10, "ESC: Close Modal");
}

static bool SynthesizerViewOnKeyEvent(KeyHandler *_self, int code)
{
    SynthesizerView *self = _self;

    __Dump__C(code);

    switch (code) {
    case '<':
        if (-2 < self->octave) {
            --self->octave;
            ViewInvalidate(self);
        }
        return true;
    case '>':
        if (8 > self->octave) {
            ++self->octave;
            ViewInvalidate(self);
        }
        return true;
    }

    return false;
}

static void SynthesizerViewSetNextKeyHandler(KeyHandler *_self, KeyHandler *next)
{
    SynthesizerView *self = _self;
    self->nextKeyHandler = next;
}

static KeyHandler *SynthesizerViewGetNextKeyHandler(KeyHandler *_self)
{
    SynthesizerView *self = _self;
    return self->nextKeyHandler;
}

static void SynthesizerViewDestroy(View *_self)
{
    SynthesizerView *self = _self;
    ControllerUnregisterTimer(self->controller, self);
    MixerRemoveObserver(self->mixer, self);
    ViewNodeDestroy(self->node);
    free(self);
}

static void SynthesizerViewTimerCallback(void *receiver, int64_t msec)
{
    __Dump__L(msec);
}

static void SynthesizerViewMixerOnChannelStatusChange(void *receiver, MixerChannel *channel, MixerChannelStatusKind kind)
{
    SynthesizerView *self = receiver;
    if (self->mixerChannel == channel) {
        if (kind == MixerChannelStatusKindPreset) {
            ViewInvalidate(self);
        }
    }
}

static void SynthesizerViewMixerOnAvailableMidiSourceChange(void *receiver, NAArray *descriptions)
{
}

static void SynthesizerViewMixerOnLevelUpdate(void *receiver)
{
}

static MixerObserverCallbacks SynthesizerViewMixerObserverCallbacks = {
    SynthesizerViewMixerOnChannelStatusChange,
    SynthesizerViewMixerOnAvailableMidiSourceChange,
    SynthesizerViewMixerOnLevelUpdate,
};
