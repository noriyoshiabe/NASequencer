#include "MMLDriver.h"
#include "MMLParser.h"
#include "MMLASTDumper.h"
#include "MMLASTAnalyzer.h"
#include "MMLSEMDumper.h"
#include "MMLSEMAnalyzer.h"
#include "NAIO.h"

#include <stdlib.h>

typedef struct _MMLDriver {
    Driver driver;
    ParseContext *context;
} MMLDriver;

static const AnalyzerFactory AnalyzerFactories[] = {
#ifdef DEBUG
    MMLASTDumperCreate,
#endif
    MMLASTAnalyzerCreate,
#ifdef DEBUG
    MMLSEMDumperCreate,
#endif
    MMLSEMAnalyzerCreate,
};

static const int AnalyzerCount = sizeof(AnalyzerFactories) / sizeof(AnalyzerFactories[0]);

static void *MMLDriverParse(void *_self, const char *filepath, ParseInfo **info)
{
    MMLDriver *self =  _self;

    char *fullpath = NAIOGetRealPath(filepath);

    DSLParser *parser = MMLParserCreate(self->context);
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

static void MMLDriverDestroy(void *self)
{
    free(self);
}

Driver *MMLDriverCreate(ParseContext *context)
{
    MMLDriver *self = calloc(1, sizeof(MMLDriver));
    self->context = context;
    self->driver.parse = MMLDriverParse;
    self->driver.destroy = MMLDriverDestroy;
    return (Driver *)self;
}
