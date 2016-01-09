#include "MMLParser.h"
#include "MMLPreprocessor.h"
#include "MMLAST.h"
#include "NAIO.h"
#include "MML_yacc.h"
#include "MML_lex.h"
#include "NASet.h"
#include "NAMap.h"
#include "NAStack.h"

#include <stdlib.h>
#include <libgen.h>

typedef struct _MMLParser {
    DSLParser parser;
    ParseContext *context;
    char *currentFilepath;
} MMLParser;

extern int MML_parse(yyscan_t scanner, void **node);

static Node *MMLParserParse(void *_self, const char *filepath)
{
    MMLParser *self = _self; 

    MMLPreprocessor *preprocessor = MMLPreprocessorCreate(self->context);
    FILE *fp = MMLPreprocessorScanFile(preprocessor, filepath);
    MMLPreprocessorDestroy(preprocessor);

    yyscan_t scanner;
    MML_lex_init_extra(self, &scanner);
    YY_BUFFER_STATE state = MML__create_buffer(fp, YY_BUF_SIZE, scanner);
    MML__switch_to_buffer(state, scanner);

    Node *node = NULL;
    MML_parse(scanner, (void **)&node);

    MML__delete_buffer(state, scanner);
    MML_lex_destroy(scanner);

    fclose(fp);

    return node;
}

static void MMLParserDestroy(void *_self)
{
    MMLParser *self = _self;
    free(self);
}

DSLParser *MMLParserCreate(ParseContext *context)
{
    MMLParser *self = calloc(1, sizeof(MMLParser));
    self->context = context;
    self->parser.parse = MMLParserParse;
    self->parser.destroy = MMLParserDestroy;
    return (DSLParser *)self;
}

void MMLParserSetCurrentFilepath(void *_self, const char *filepath)
{
    MMLParser *self = _self;
    self->currentFilepath = self->context->appendFile(self->context, filepath);
}

char *MMLParserGetCurrentFilepath(void *_self)
{
    MMLParser *self = _self;
    return self->currentFilepath;
}

void MMLParserSyntaxError(void *_self, FileLocation *location, const char *token)
{
    MMLParser *self = _self; 
    self->context->appendError(self->context, location, GeneralParseErrorSyntaxError, token, NULL);
}

void MMLParserUnExpectedEOF(void *_self, FileLocation *location)
{
    MMLParser *self = _self; 
    self->context->appendError(self->context, location, MMLParseErrorUnexpectedEOF, NULL);
}
