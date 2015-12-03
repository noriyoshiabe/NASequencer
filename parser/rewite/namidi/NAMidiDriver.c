#include "NAMidiDriver.h"
#include "NAMidiParser.h"
#include "NAMidiASTAnalyzer.h"
#include "NAMidiSEMAnalyzer.h"

#include <stdlib.h>

typedef struct _NAMidiDriver {
    Driver driver;
    ParseContext *context;
} NAMidiDriver;

static const AnalyzerFactory AnalyzerFactories[] = {
    NAMidiASTAnalyzerCreate,
    NAMidiSEMAnalyzerCreate,
};

static const int AnalyzerCount = sizeof(AnalyzerFactories) / sizeof(AnalyzerFactories[0]);

static void *NAMidiDriverParse(void *_self, const char *filepath, ParseInfo **info)
{
    NAMidiDriver *self =  _self;

    DSLParser *parser = NAMidiParserCreate(self->context);
    Node *node = parser->parse(parser, filepath);
    parser->destroy(parser);

    if (!node) {
        goto EXIT;
    }

    for (int i = 0; i < AnalyzerCount; ++i) {
        Analyzer *analyzer = AnalyzerFactories[i](self->context);
        Node *_node = analyzer->process(analyzer, node);
        analyzer->destroy(analyzer);
        NodeRelease(node);
        node = _node;
    }

    NodeRelease(node);

EXIT:
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
