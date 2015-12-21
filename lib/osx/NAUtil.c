#include "NAUtil.h"

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

typedef struct _fmem_t {
    int pos;
    int size;
    char *buffer;
    int capacity;
    int allocationUnit;
} fmem_t;

static int __read(void *cookie, char *buffer, int size)
{
    fmem_t *mem = cookie;
    int available = mem->size - mem->pos;

    if (size > available) {
        size = available;
    }

    memcpy(buffer, mem->buffer + mem->pos, size);
    mem->pos += size;

    return size;
}

static int __write(void *cookie, const char *buffer, int size)
{
    fmem_t *mem = cookie;
    int available = mem->capacity - mem->pos;

    while (size > available) {
        mem->capacity += mem->allocationUnit;
        mem->buffer = realloc(mem->buffer, mem->capacity);
        available = mem->capacity - mem->pos;
    }

    memcpy(mem->buffer + mem->pos, buffer, size);
    mem->pos += size;
    mem->size += size;

    return size;
}

static fpos_t __seek(void *cookie, fpos_t offset, int whence)
{
    fmem_t *mem = cookie;

    switch (whence) {
    case SEEK_SET:
        mem->pos = (int)offset;
        break;
    case SEEK_CUR:
        mem->pos += (int)offset;
        break;
    case SEEK_END:
        mem->pos = mem->size + (int)offset;
        break;
    default:
        return -1;
    }

    if (0 > mem->pos) {
        mem->pos = 0;
    }
    else if (mem->size < mem->pos) {
        return -1;
    }

    return (fpos_t)mem->pos;
}

static int __close(void *cookie)
{
    fmem_t *mem = cookie;
    free(mem->buffer);
    free(cookie);
    return 0;
}

FILE *NAUtilCreateMemoryStream(int allocationUnit)
{
    fmem_t *mem = calloc(1, sizeof(fmem_t));

    mem->allocationUnit = allocationUnit;
    mem->capacity += mem->allocationUnit;
    mem->buffer = malloc(mem->capacity);

    return funopen(mem, __read, __write, __seek, __close);
}
