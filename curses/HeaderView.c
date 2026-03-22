#include "Interface.h"
#include "View.h"
#include "ViewNode.h"
#include "HeaderView.h"
#include "NAIO.h"

#include <string.h>
#include <stdlib.h>

struct _HeaderView {
    const InterfaceVtbl *vtbl;
    ViewNode *node;
    NAMidi *namidi;
    Player *player;
    char *filepath;
    Sequence *sequence;
};

static ViewNode *HeaderViewGetNode(View *self);
static void HeaderViewDraw(View *self, Size size);
static void HeaderViewDestroy(View *self);

static const ViewVtbl HeaderViewViewVtbl = {
    .getNode = HeaderViewGetNode,
    .draw = HeaderViewDraw,
    .destroy = HeaderViewDestroy,
};

static void *HeaderViewQueryInteface(void *self, IID iid)
{
    if (iid == IIDView)
        return (void*)&HeaderViewViewVtbl;
    return NULL;
}

static const InterfaceVtbl HeaderViewInterfaceVtbl = {
    .queryInterface = HeaderViewQueryInteface,
};

static NAMidiObserverCallbacks HeaderViewNAMidiObserverCallbacks;
static PlayerObserverCallbacks HeaderViewPlayerObserverCallbacks;

HeaderView *HeaderViewCreate(NAMidi *namidi)
{
    HeaderView *self = calloc(1, sizeof(HeaderView));
    self->vtbl = &HeaderViewInterfaceVtbl;
    self->node = ViewNodeCreate(self);

    self->namidi = namidi;
    NAMidiAddObserver(self->namidi, self, &HeaderViewNAMidiObserverCallbacks);

    self->player = NAMidiGetPlayer(namidi);
    PlayerAddObserver(self->player, self, &HeaderViewPlayerObserverCallbacks);

    return self;
}

static ViewNode *HeaderViewGetNode(View *_self)
{
    HeaderView *self = (HeaderView *)_self;
    return self->node;
}

static void HeaderViewDraw(View *_self, Size size)
{
    HeaderView *self = (HeaderView *)_self;

    char line[size.width + 1];
    char name[64] = {};

    snprintf(name, sizeof(name), "%s", self->filepath);
    if (self->sequence->title) {
        snprintf(name, sizeof(name), "%s - %s", name, self->sequence->title);
    }

    const char *markerText = PlayerGetMarkerText(self->player);

    char section[64] = {};
    snprintf(section, sizeof(section), "Section %s", markerText);

    snprintf(line, sizeof(line), "%-*s", size.width, name);
    snprintf(line + size.width - strlen(section), strlen(section) + 1, "%s", section);
    ViewPrintf(self, 0, 0, line);

    float tempo = PlayerGetTempo(self->player);
    TimeSign ts = PlayerGetTimeSign(self->player);
    Location loc = PlayerGetLocation(self->player);
    int usec = PlayerGetUsec(self->player);
    int msec = usec / 1000;
    int sec = usec / 1000000;

    char timeTable[64] = {};
    snprintf(timeTable, sizeof(timeTable), "Tempo %.2f Time Signature %d/%d", tempo, ts.numerator, ts.denominator);
    char timeLocation[35] = {};
    snprintf(timeLocation, sizeof(timeLocation), "Location %03d:%02d:%03d Time %02d:%02d:%03d", loc.m, loc.b, loc.t, sec / 60, sec % 60, msec % 1000);

    snprintf(line, sizeof(line), "%-*s", size.width, timeTable);
    snprintf(line + size.width - 34, 35, "%s", timeLocation);
    
    ViewPrintf(self, 0, 1, line);

    memset(line, '=', sizeof(line) - 1);
    line[sizeof(line)] = '\0';
    ViewPrintf(self, 0, 2, line);
}

static void HeaderViewDestroy(View *_self)
{
    HeaderView *self = (HeaderView *)_self;

    if (self->filepath) {
        free(self->filepath);
    }
    if (self->sequence) {
        SequenceRelease(self->sequence);
    }

    NAMidiRemoveObserver(self->namidi, self);
    ViewNodeDestroy(self->node);
    free(self);
}

static void HeaderViewNAMidiOnBeforeParse(void *receiver, bool fileChanged)
{
}

static void HeaderViewNAMidiOnParseFinish(void *receiver, Sequence *sequence, ParseInfo *info)
{
    HeaderView *self = receiver;

    if (self->filepath) {
        free(self->filepath);
    }
    if (0 < NAArrayCount(info->filepaths)) {
        self->filepath = strdup(NAIOGetLastPathComponent(NAArrayGetValueAt(info->filepaths, 0)));
    } else {
        self->filepath = strdup("");
    }

    if (self->sequence) {
        SequenceRelease(self->sequence);
    }
    self->sequence = SequenceRetain(sequence);

    ViewInvalidate(self);
}

static NAMidiObserverCallbacks HeaderViewNAMidiObserverCallbacks = {
    HeaderViewNAMidiOnBeforeParse,
    HeaderViewNAMidiOnParseFinish,
};

static void HeaderViewPlayerOnNotifyClock(void *receiver, int tick, int64_t usec, Location location)
{
    HeaderView *self = receiver;
    ViewInvalidate(self);
}

static void HeaderViewPlayerOnNotifyEvent(void *receiver, PlayerEvent event)
{
}

static void HeaderViewPlayerOnSendNoteOn(void *receiver, NoteEvent *event)
{
}

static void HeaderViewPlayerOnSendNoteOff(void *receiver, NoteEvent *event)
{
}

static PlayerObserverCallbacks HeaderViewPlayerObserverCallbacks = {
    HeaderViewPlayerOnNotifyClock,
    HeaderViewPlayerOnNotifyEvent,
    HeaderViewPlayerOnSendNoteOn,
    HeaderViewPlayerOnSendNoteOff
};
