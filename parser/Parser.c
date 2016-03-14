#include "Parser.h"
#include "NAMidiDriver.h"
#include "ABCDriver.h"
#include "MMLDriver.h"

#include "NAIO.h"
#include <stdlib.h>
#include <string.h>

struct _Parser {
    SequenceBuilder *builder;
    ParseContext *context;
};

Parser *ParserCreate(SequenceBuilder *builder, const char *includePath)
{
    Parser *self = calloc(1, sizeof(Parser));
    self->builder = builder;
    self->context = ParseContextCreate(builder, includePath);
    return self;
}

void ParserDestroy(Parser *self)
{
    ParseContextDestroy(self->context);
    free(self);
}

static Driver *ParserDriverCreate(Parser *self, const char *filepath)
{
    const char *ext = NAIOGetFileExtenssion(filepath);

    const struct {
        const char *ext;
        DriverFactory factory;
    } table[] = {
        {"nas", NAMidiDriverCreate},
        {"abc", ABCDriverCreate},
        {"mml", MMLDriverCreate},
    };

    for (int i = 0; i < sizeof(table) / sizeof(table[0]); ++i) {
        if (0 == strcasecmp(table[i].ext, ext)) {
            return table[i].factory(self->context);
        }
    }

    self->context->appendError(self->context, NULL, GeneralParseErrorUnsupportedFileType, ext, NULL);
    return NULL;
}

void *ParserParseFile(Parser *self, const char *filepath, ParseInfo **info)
{
    Driver *driver = ParserDriverCreate(self, filepath);
    if (!driver) {
        return self->context->buildResult(self->context, info);
    }

    void *ret = driver->parse(driver, filepath, info);
    driver->destroy(driver);
    return ret;
}
