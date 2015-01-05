#pragma once

#include <CoreFoundation/CoreFoundation.h>

typedef struct __NAType NAType;

extern const void *NARetain(const void *self);
extern void NARelease(const void *self);
extern uint32_t NAHash(const void *self);
extern Boolean NAEqual(const void *self, const void *to);

extern const CFArrayCallBacks __CFArrayCallBacks;
