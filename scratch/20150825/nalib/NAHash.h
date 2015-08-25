#pragma once

#include <stdint.h>

typedef uint32_t (*NAHash)(void *value);
extern uint32_t NAHashAddress(void *value);
extern uint32_t NAHashCString(void *value);
