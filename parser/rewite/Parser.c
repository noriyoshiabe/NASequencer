#include "Parser.h"
#include "NAMidiDriver.h"

#include "NAUtil.h"
#include <stdlib.h>
#include <string.h>

struct _Parser {
    SequenceBuilder *builder;
    ParseContext *context;
};

Parser *ParserCreate(SequenceBuilder *builder)
{
    Parser *self = calloc(1, sizeof(Parser));
    self->builder = builder;
    self->context = ParseContextCreate(builder);
    return self;
}

void ParserDestroy(Parser *self)
{
    ParseContextDestroy(self->context);
    free(self);
}

static Driver *ParserDriverCreate(Parser *self, const char *filepath)
{
    const char *ext = NAUtilGetFileExtenssion(filepath);

    const struct {
        const char *ext;
        DriverFactory factory;
    } table[] = {
        {"namidi", NAMidiDriverCreate},
    };

    for (int i = 0; i < sizeof(table) / sizeof(table[0]); ++i) {
        if (0 == strcmp(table[i].ext, ext)) {
            return table[i].factory(self->context);
        }
    }

    self->context->appendError(self->context, NULL, GeneralParseErrorUnsupportedFileType, ext, NULL);
    return NULL;
}

ParseResult *ParserParseFile(Parser *self, const char *filepath)
{
    Driver *driver = ParserDriverCreate(self, filepath);
    if (!driver) {
        return self->context->buildResult(self->context);
    }

    ParseResult *ret = driver->parse(driver, filepath);
    driver->destroy(driver);
    return ret;
}
