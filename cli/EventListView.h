#pragma once

#include "NAMidi.h"

typedef struct _EventListView EventListView;

extern EventListView *EventListViewCreate(NAMidi *namidi);
extern void EventListViewDestroy(EventListView *self);
extern void EventListViewSetChannel(EventListView *self, int channel);
extern void EventListViewSetFrom(EventListView *self, int from);
extern void EventListViewSetLength(EventListView *self, int length);
extern void EventListViewRender(EventListView *self);
extern void EventListViewSetSequence(EventListView *self, Sequence *sequence);
