#include "Interface.h"
#include "View.h"
#include "ViewNode.h"
#include "ChannelView.h"

#include <string.h>
#include <stdlib.h>

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
    // TODO

    ChannelView *self = (ChannelView *)_self;
    ViewPrintf(self, 0, 1, "Channel: %d", self->channel);

    Level level = MixerChannelGetLevel(self->mixerChannel);
    ViewPrintf(self, 0, 2, "L: %d R: %d", level.L, level.R);
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
