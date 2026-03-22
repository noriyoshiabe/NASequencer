#pragma once

#include "Interface.h"
#include <stdbool.h>

extern const IID IIDKeyHandler;

typedef void KeyHandler;

typedef struct {
    bool (*onKeyEvent)(KeyHandler *self, int code);
    void (*setNextKeyHandler)(KeyHandler *self, KeyHandler *next);
} KeyHandlerVtbl, *IKeyHandler;

extern bool KeyHandlerHandleKeyEvent(KeyHandler *self, int code);
extern void KeyHandlerSetNextKeyHandler(KeyHandler *self, KeyHandler *next);
