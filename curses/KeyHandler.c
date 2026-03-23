#include "KeyHandler.h"
#include "Debug.h"

const IID IIDKeyHandler = InterfaceID("KeyHandler");

bool KeyHandlerHandleKeyEvent(KeyHandler *self, int code)
{
    return QueryInterface(self, IIDKeyHandler, IKeyHandler)->onKeyEvent(self, code);
}

void KeyHandlerSetNextKeyHandler(KeyHandler *self, KeyHandler *next)
{
    QueryInterface(self, IIDKeyHandler, IKeyHandler)->setNextKeyHandler(self, next);
}

KeyHandler *KeyHandlerGetNextKeyHandler(KeyHandler *self)
{
    return QueryInterface(self, IIDKeyHandler, IKeyHandler)->getNextKeyHandler(self);
}
