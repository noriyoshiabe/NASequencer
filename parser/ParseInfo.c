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
    if (self->location.filepath) {
        free(self->location.filepath);
    }

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

        free(self);
    }
}

#include "ParseErrorCode.h"
#include "NAMidiParser.h"
#include "ABCParser.h"
#include "MMLParser.h"
#include "NAStringBuffer.h"
#include "NACString.h"

#include <string.h>
#include <ctype.h>

char *ParseErrorFormattedString(const ParseError *error)
{
    NAStringBuffer *buffer = NAStringBufferCreate(1024);

    NAStringBufferAppendFormat(buffer, "[ERROR:%d] ", error->code);

    const char *errorString = ParseErrorCode2String(error->code);
    const char *search = "ParseError";
    const char *camel = strstr(errorString, search) + strlen(search);

    char *message = NACStringCamel2Readable(camel);
    NAStringBufferAppendString(buffer, message);
    free(message);

    NAStringBufferAppendString(buffer, ". ");

    bool needseparator = false;
    NAIterator *iterator = NAArrayGetIterator(error->infos);
    while (iterator->hasNext(iterator)) {
        NAStringBufferAppendFormat(buffer, "%s\"%s\"", needseparator ? "," : "", iterator->next(iterator));
        needseparator = true;
    }

    NAStringBufferAppendFormat(buffer, " - %s:%d:%d", error->location.filepath, error->location.line, error->location.column);

    char *ret = NAStringBufferRetriveCString(buffer);
    NAStringBufferDestroy(buffer);
    return ret;
}
