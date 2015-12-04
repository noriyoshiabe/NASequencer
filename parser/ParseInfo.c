#include "ParseInfo.h"

#include <stdlib.h>

ParseError *ParseErrorCreate()
{
    ParseError *self = calloc(1, sizeof(ParseError));
    self->infos = NAArrayCreate(4, NADescriptionCString);
    return self;
}

void ParseErrorDestroy(ParseError *self)
{
    NAArrayTraverse(self->infos, free);
    NAArrayDestroy(self->infos);
    free(self);
}

typedef struct _ParseInfoImpl {
    ParseInfo info;
    int refCount;
} ParseInfoImpl;

ParseInfo *ParseInfoCreate()
{
    ParseInfoImpl *self = calloc(1, sizeof(ParseInfoImpl));
    self->refCount = 1;
    return (ParseInfo *)self;
}

ParseInfo *ParseInfoRetain(ParseInfo *_self)
{
    ParseInfoImpl *self = (ParseInfoImpl *)_self;
    ++self->refCount;
    return (ParseInfo *)self;
}

void ParseInfoRelease(ParseInfo *_self)
{
    ParseInfoImpl *self = (ParseInfoImpl *)_self;
    if (0 == --self->refCount) {
        if (self->info.filepaths) {
            NAArrayTraverse(self->info.filepaths, free);
            NAArrayDestroy(self->info.filepaths);
        }

        if (self->info.errors) {
            NAArrayTraverse(self->info.errors, ParseErrorDestroy);
            NAArrayDestroy(self->info.errors);
        }
    }

    free(self);
}
