#include "ABCDriver.h"
#include "ABCParser.h"
#include "ABCASTDumper.h"
#include "ABCASTAnalyzer.h"
#include "ABCSEMDumper.h"
//#include "ABCSEMAnalyzer.h"
#include "NAUtil.h"

#include <stdlib.h>

typedef struct _ABCDriver {
    Driver driver;
    ParseContext *context;
} ABCDriver;

static const AnalyzerFactory AnalyzerFactories[] = {
#if 1
    ABCASTDumperCreate,
#endif
    ABCASTAnalyzerCreate,
#if 1
    ABCSEMDumperCreate,
#endif
    //ABCSEMAnalyzerCreate,
};

static const int AnalyzerCount = sizeof(AnalyzerFactories) / sizeof(AnalyzerFactories[0]);

static void *ABCDriverParse(void *_self, const char *filepath, ParseInfo **info)
{
    ABCDriver *self =  _self;

    char *fullpath = NAUtilGetRealPath(filepath);

    DSLParser *parser = ABCParserCreate(self->context);
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

static void ABCDriverDestroy(void *self)
{
    free(self);
}

Driver *ABCDriverCreate(ParseContext *context)
{
    ABCDriver *self = calloc(1, sizeof(ABCDriver));
    self->context = context;
    self->driver.parse = ABCDriverParse;
    self->driver.destroy = ABCDriverDestroy;
    return (Driver *)self;
}
