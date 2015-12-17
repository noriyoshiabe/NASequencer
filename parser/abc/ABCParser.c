#include "ABCParser.h"
#include "ABCAST.h"
#include "NAUtil.h"
#include "ABC_yacc.h"
#include "ABC_lex.h"
#include "NASet.h"
#include "NAMap.h"

#include <stdlib.h>
#include <libgen.h>

typedef struct _ABCParser {
    DSLParser parser;
    ParseContext *context;
    NASet *readingFileSet;
    NAMap *includedNodeMap;
} ABCParser;

extern int ABC_parse(yyscan_t scanner, const char *filepath, void **node);

static Node *ABCParserParseInternal(ABCParser *self, const char *filepath)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        return NULL;
    }

    self->context->appendFile(self->context, filepath);

    NASetAdd(self->readingFileSet, (char *)filepath);

    yyscan_t scanner;
    ABC_lex_init_extra(self, &scanner);
    YY_BUFFER_STATE state = ABC__create_buffer(fp, YY_BUF_SIZE, scanner);
    ABC__switch_to_buffer(state, scanner);

    Node *node = NULL;
    ABC_parse(scanner, filepath, (void **)&node);

    ABC__delete_buffer(state, scanner);
    ABC_lex_destroy(scanner);
    fclose(fp);

    NASetRemove(self->readingFileSet, (char *)filepath);

    return node;
}

static Node *ABCParserParse(void *_self, const char *filepath)
{
    ABCParser *self = _self; 

    Node *node = ABCParserParseInternal(self, filepath);
    if (!node) {
        self->context->appendError(self->context, NULL, GeneralParseErrorFileNotFound, filepath, NULL);
    } 

    return node;
}

static void ABCParserDestroy(void *_self)
{
    ABCParser *self = _self;
    NASetDestroy(self->readingFileSet);
    NAMapDestroy(self->includedNodeMap);
    free(self);
}

DSLParser *ABCParserCreate(ParseContext *context)
{
    ABCParser *self = calloc(1, sizeof(ABCParser));
    self->context = context;
    self->parser.parse = ABCParserParse;
    self->parser.destroy = ABCParserDestroy;
    self->readingFileSet = NASetCreate(NAHashCString, NADescriptionCString);
    self->includedNodeMap = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    return (DSLParser *)self;
}

Node *ABCParserParseIncludeFile(void *_self, FileLocation *location, const char *includeFile)
{
    ABCParser *self = _self; 

    const char *ext = NAUtilGetFileExtenssion(includeFile);
    if (0 != strcmp("abh", ext)) {
        self->context->appendError(self->context, location, ABCParseErrorUnsupportedFileTypeInclude, ext, includeFile, NULL);
        return NULL;
    }

    char *directory = dirname(location->filepath);
    char *fullpath = NAUtilBuildPathWithDirectory(directory, includeFile);

    if (NASetContains(self->readingFileSet, fullpath)) {
        self->context->appendError(self->context, location, ABCParseErrorCircularFileInclude, includeFile, NULL);
        free(fullpath);
        return NULL;
    }

    Node *node = NAMapGet(self->includedNodeMap, fullpath);
    if (node) {
        free(fullpath);
        return NodeRetain(node);
    }

    node = ABCParserParseInternal(self, fullpath);
    if (!node) {
        self->context->appendError(self->context, location, ABCParseErrorIncludeFileNotFound, includeFile, NULL);
        free(fullpath);
        return NULL;
    }

    ASTInstInclude *include = (ASTInstInclude *)node;
    include->fullpath = fullpath;
    NAMapPut(self->includedNodeMap, include->fullpath, include);
    return node;
}

void ABCParserSyntaxError(void *_self, FileLocation *location, const char *token)
{
    ABCParser *self = _self; 
    self->context->appendError(self->context, location, GeneralParseErrorSyntaxError, token, NULL);
}
