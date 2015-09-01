#pragma once

typedef struct _NAStack NAStack;

extern NAStack *NAStackCreate(int initialSize);
extern void NAStackDestroy(NAStack *self);
extern void NAStackPush(NAStack *self, void *value);
extern void *NAStackPop(NAStack *self);
extern int NAStackCount(NAStack *self);
