#include "NAMidiDriver.h"
#include "NAMidiParser.h"
#include "NAMidiASTDumper.h"
#include "NAMidiASTAnalyzer.h"
#include "NAMidiSEMDumper.h"
#include "NAMidiSEMAnalyzer.h"
#include "NAIO.h"

#include <stdlib.h>

typedef struct _NAMidiDriver {
    Driver driver;
    ParseContext *context;
} NAMidiDriver;

static const AnalyzerFactory AnalyzerFactories[] = {
#ifdef DEBUG
    NAMidiASTDumperCreate,
#endif
    NAMidiASTAnalyzerCreate,
#ifdef DEBUG
    NAMidiSEMDumperCreate,
#endif
    NAMidiSEMAnalyzerCreate,
};

static const int AnalyzerCount = sizeof(AnalyzerFactories) / sizeof(AnalyzerFactories[0]);

static void *NAMidiDriverParse(void *_self, const char *filepath, ParseInfo **info)
{
    NAMidiDriver *self =  _self;

    char *fullpath = NAIOGetRealPath(filepath);

    DSLParser *parser = NAMidiParserCreate(self->context);
    Node *node = parser->parse(parser, fullpath);
    parser->destroy(parser);

    if (!node) {
        goto EXIT;
    }

    const AnalyzerFactory *factory = AnalyzerFactories;
    while (node) {
        Analyzer *analyzer = (*factory++)(self->context);
        Node *_node = analyzer->process(analyzer->self, node);
        analyzer->destroy(analyzer->self);
        NodeRelease(node);
        node = _node;
    }

EXIT:
    free(fullpath);

    return self->context->buildResult(self->context, info);
}

static void NAMidiDriverDestroy(void *self)
{
    free(self);
}

Driver *NAMidiDriverCreate(ParseContext *context)
{
    NAMidiDriver *self = calloc(1, sizeof(NAMidiDriver));
    self->context = context;
    self->driver.parse = NAMidiDriverParse;
    self->driver.destroy = NAMidiDriverDestroy;
    return (Driver *)self;
}
