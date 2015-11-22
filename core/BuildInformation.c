#include "BuildInformation.h"

#include <stdlib.h>

typedef struct _BuildInformationImpl {
    BuildInformation info;
    int refCount;
} BuildInformationImpl;

BuildInformation *BuildInformationCreate()
{
    BuildInformationImpl *self = calloc(1, sizeof(BuildInformationImpl));
    self->refCount = 1;
    self->info.filepaths = NAArrayCreate(4, NADescriptionCString);
    self->info.errors = NAArrayCreate(4, NULL);
    return (BuildInformation *)self;
}

BuildInformation *BuildInformationRetain(BuildInformation *_self)
{
    BuildInformationImpl *self = (BuildInformationImpl *)_self;
    ++self->refCount;
    return _self;
}

static void BuildInformationParseErrorDestroy(ParseError *error)
{
    free((char *)error->location.filepath);
    free(error);
}

void BuildInformationRelease(BuildInformation *_self)
{
    BuildInformationImpl *self = (BuildInformationImpl *)_self;
    if (0 == --self->refCount) {
        NAArrayTraverse(self->info.filepaths, free);
        NAArrayTraverse(self->info.errors, BuildInformationParseErrorDestroy);
        NAArrayDestroy(self->info.filepaths);
        NAArrayDestroy(self->info.errors);
        free(self);
    }
}
