#include "KeyHandler.h"
#include "Debug.h"

const IID IIDKeyHandler = InterfaceID("KeyHandler");

bool KeyHandlerHandleKeyEvent(KeyHandler *self, int code)
{
    return QueryInterface(self, IIDKeyHandler, IKeyHandler)->onKeyEvent(self, code);
}

void KeyHandlerSetNextKeyHandler(KeyHandler *self, KeyHandler *next)
{
    if (QueryInterface(next, IIDKeyHandler, IKeyHandler)) {
        QueryInterface(self, IIDKeyHandler, IKeyHandler)->setNextKeyHandler(self, next);
    }
}
