#pragma once

#include "NAMidi.h"
#include "Controller.h"

typedef struct _MainView MainView;

extern MainView *MainViewCreate(NAMidi *namidi);
extern bool MainViewGetChannelExists(MainView *self, int channel);
extern void MainViewSetController(MainView *self, Controller *controller);
