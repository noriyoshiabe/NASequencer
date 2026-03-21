#pragma once

#include <stdint.h>

typedef uintptr_t IID;

typedef struct {
    void *(*queryInterface)(void *self, IID iid);
} InterfaceVtbl;

typedef struct {
    InterfaceVtbl *vtbl;
} Interface;

#define InterfaceID(str) (IID)str
#define QueryInterface(self, iid, type) ((type)(((Interface *)self)->vtbl->queryInterface(self, iid)))
