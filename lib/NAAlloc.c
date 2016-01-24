#define __ALLOCATION_CHECK__
#ifdef __ALLOCATION_CHECK__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dlfcn.h>
#include <execinfo.h>

#include "NAMap.h"

typedef struct _Stat {
    void *ptr;
    void *buffer[16];
    int count;
} Stat;

static NAMap *map;

static void *(*libc_malloc)(size_t size);
static void *(*libc_calloc)(size_t nmemb, size_t size);
static void *(*libc_realloc)(void *ptr, size_t new_size);
static char *(*libc_strdup)(const char *s);
static char *(*libc_strndup)(const char *s, size_t n);
static void (*libc_free)(void *ptr);

static char *(*libedit_readline)(const char *prompt);

static void putStat(void *ptr, void **buffer, int count)
{
    Stat *stat = libc_malloc(sizeof(Stat));
    stat->ptr = ptr;
    for (int i = 0; i < count; ++i) {
        stat->buffer[i] = buffer[i];
    }
    stat->count = count;

    NAMapPut(map, ptr, stat);
}

static void removeStat(void *ptr, void **buffer, int count)
{
    Stat *stat = NAMapRemove(map, ptr);
    if (!stat) {
        printf("\nmemory stat not found\n");
        printf("--------------------------------------------------------\n");

        char **symbols = backtrace_symbols(buffer, count);
        for (int i = 0; i < count; ++i) {
            printf("%s\n", symbols[i]);
        }

        libc_free(symbols);
        abort();
    }
}

static void exitMemoryStats()
{
    printf("\nexit memory stats.\n");
    printf("--------------------------------------------------------\n");

    bool needSepalator = false;
    NAIterator *iterator = NAMapGetIterator(map);
    while (iterator->hasNext(iterator)) {
        NAMapEntry *entry = iterator->next(iterator);
        Stat *stat = entry->value;

        if (needSepalator) {
            printf("--------------------------------------------------------\n");
        }
        needSepalator = true;

        char **symbols = backtrace_symbols(stat->buffer, stat->count);
        for (int i = 0; i < stat->count; ++i) {
            printf("%s\n", symbols[i]);
        }

        libc_free(symbols);
        libc_free(stat);
    }
    
    NAMapDestroy(map);
}

void *malloc(size_t size)
{
    void *buffer[16];
    int count = backtrace(buffer, 16);

    void *ret = libc_malloc(size);
    putStat(ret, buffer, count);
    return ret;
}

void *calloc(size_t nmemb, size_t size)
{
    void *buffer[16];
    int count = backtrace(buffer, 16);

    void *ret = libc_calloc(nmemb, size);
    putStat(ret, buffer, count);
    return ret;
}

void *realloc(void *ptr, size_t size)
{
    void *buffer[16];
    int count = backtrace(buffer, 16);

    if (ptr) {
        removeStat(ptr, buffer, count);
    }

    void *ret = libc_realloc(ptr, size);
    putStat(ret, buffer, count);
    return ret;
}

char *strdup(const char *s)
{
    void *buffer[16];
    int count = backtrace(buffer, 16);

    char *ret = libc_strdup(s);
    putStat(ret, buffer, count);
    return ret;
}

char *strndup(const char *s, size_t n)
{
    void *buffer[16];
    int count = backtrace(buffer, 16);

    char *ret = libc_strndup(s, n);
    putStat(ret, buffer, count);
    return ret;
}

void free(void *ptr)
{
    if (ptr) {
        void *buffer[16];
        int count = backtrace(buffer, 16);
        removeStat(ptr, buffer, count);
    }

    libc_free(ptr);
}

char *readline(const char *prompt)
{
    void *buffer[16];
    int count = backtrace(buffer, 16);

    char *ret = libedit_readline(prompt);
    putStat(ret, buffer, count);
    return ret;
}

__attribute__((constructor))
static void initialize()
{
    libc_calloc = dlsym(RTLD_NEXT, "calloc");
    libc_malloc = dlsym(RTLD_NEXT, "malloc");
    libc_realloc = dlsym(RTLD_NEXT, "realloc");
    libc_strdup = dlsym(RTLD_NEXT, "strdup");
    libc_strndup = dlsym(RTLD_NEXT, "strndup");
    libc_free = dlsym(RTLD_NEXT, "free");

    libedit_readline = dlsym(RTLD_NEXT, "readline");

    map = NAMapCreateWithAllocator(NAHashAddress, NADescriptionAddress, NADescriptionAddress, libc_calloc, libc_free);
    atexit(exitMemoryStats);
}

#endif
