#pragma once

typedef struct _Controller Controller;

extern Controller *ControllerCreate();
extern void ControllerDestroy(Controller *controller);
