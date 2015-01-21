#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct __NAVtblEntry {
    const char *typeID;
    void *vtbl;
} NAVtblEntry;

typedef struct __NAClass {
    const char *typeID;
    size_t size;
    NAVtblEntry *pvEntry;
} NAClass;

typedef struct __NAType {
    const NAClass *clazz;
    int32_t refCount;
} NAType;

typedef struct __NATypeVtbl {
    void *(*init)(void *self, ...);
    void (*destroy)(void *self);
    uint32_t (*hash)(const void *self);
    bool (*equalTo)(const void *self, const void *to);
    int (*compare)(const void *self, const void *to);
    void *(*copy)(const void *self);
    void *(*description)(const void *self);
} NATypeVtbl;

extern const char NATypeID[];

#define NATypeNew(type, ...) ((NATypeVtbl *)__NAFindInit(&type##Class))->init(__NATypeAlloc(&type##Class), __VA_ARGS__)
#define NAVtbl(self, type) ((type##Vtbl *)__NAVtblLookup(((NAType *)self)->clazz, type##ID))

extern const void *NARetain(const void *self);
extern void NARelease(const void *self);
extern int16_t NARefCount(const void *self);

extern uint32_t NAHash(const void *self);
extern bool NAEqual(const void *self, const void *to);
extern int NACompare(const void *self, const void *to);
extern void *NACopy(const void *self);
extern void *NADescription(const void *self);

extern void *__NATypeAlloc(const NAClass *clazz);
extern void *(*__NAFindInit(const NAClass *clazz))(void *self, ...);
extern void *__NAVtblLookup(const NAClass *clazz, const char *typeID);

#define NAExportClass(type) \
    extern const NAClass type##Class; \
    extern const char type##ID[];

#define NAMacroName(_1,_2,_3,_4,_5,_6,_7,_8,_9,name, ...) name
#define NADeclareVtbl(type,inf,...) NAMacroName(__VA_ARGS__,\
        NADeclareVtbl9,\
        NADeclareVtbl8,\
        NADeclareVtbl7,\
        NADeclareVtbl6,\
        NADeclareVtbl5,\
        NADeclareVtbl4,\
        NADeclareVtbl3,\
        NADeclareVtbl2,\
        NADeclareVtbl1,\
        )(type, inf, __VA_ARGS__)

#define NADeclareVtbl9(type,inf,f1,f2,f3,f4,f5,f6,f7,f8,f9)  static inf##Vtbl __##type##__##inf##Vtbl = {f1,f2,f3,f4,f5,f6,f7,f8,f9}
#define NADeclareVtbl8(type,inf,f1,f2,f3,f4,f5,f6,f7,f8)  static inf##Vtbl __##type##__##inf##Vtbl = {f1,f2,f3,f4,f5,f6,f7,f8}
#define NADeclareVtbl7(type,inf,f1,f2,f3,f4,f5,f6,f7)  static inf##Vtbl __##type##__##inf##Vtbl = {f1,f2,f3,f4,f5,f6,f7}
#define NADeclareVtbl6(type,inf,f1,f2,f3,f4,f5,f6)  static inf##Vtbl __##type##__##inf##Vtbl = {f1,f2,f3,f4,f5,f6}
#define NADeclareVtbl5(type,inf,f1,f2,f3,f4,f5)  static inf##Vtbl __##type##__##inf##Vtbl = {f1,f2,f3,f4,f5}
#define NADeclareVtbl4(type,inf,f1,f2,f3,f4)  static inf##Vtbl __##type##__##inf##Vtbl = {f1,f2,f3,f4}
#define NADeclareVtbl3(type,inf,f1,f2,f3)  static inf##Vtbl __##type##__##inf##Vtbl = {f1,f2,f3}
#define NADeclareVtbl2(type,inf,f1,f2)  static inf##Vtbl __##type##__##inf##Vtbl = {f1,f2}
#define NADeclareVtbl1(type,inf,f1)  static inf##Vtbl __##type##__##inf##Vtbl = {f1}

#define NADeclareVtblEntry(type,...) NAMacroName(__VA_ARGS__,\
        NADeclareVtblEntry9,\
        NADeclareVtblEntry8,\
        NADeclareVtblEntry7,\
        NADeclareVtblEntry6,\
        NADeclareVtblEntry5,\
        NADeclareVtblEntry4,\
        NADeclareVtblEntry3,\
        NADeclareVtblEntry2,\
        NADeclareVtblEntry1,\
        )(type, __VA_ARGS__)

#define NADeclareVtblEntry2(type,inf1,inf2) \
    static NAVtblEntry __##type##__vEntries[] = { \
        {type##ID, &__##type##__##inf1##Vtbl}, \
        {type##ID, &__##type##__##inf2##Vtbl}, \
        {NULL, NULL} \
    }
#define NADeclareVtblEntry1(type,inf1) \
    static NAVtblEntry __##type##__vEntries[] = { \
        {type##ID, &__##type##__##inf1##Vtbl}, \
        {NULL, NULL} \
    }

#define NADeclareClass(type) \
    const char type##ID[] = #type; \
    const NAClass type##Class = { \
        type##ID, \
        sizeof(type), \
        __##type##__vEntries \
    }
