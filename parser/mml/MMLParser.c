#include "MMLParser.h"
#include "MMLAST.h"
#include "NAIO.h"
#include "MML_yacc.h"
#include "MML_lex.h"
#include "NASet.h"
#include "NAMap.h"

#include <stdlib.h>
#include <libgen.h>

typedef struct _MMLParser {
    DSLParser parser;
    ParseContext *context;
    NASet *readingFileSet;
    NAMap *includedRootNodeMap;
} MMLParser;

extern int MML_parse(yyscan_t scanner, const char *filepath, void **node);

static Node *MMLParserParseInternal(MMLParser *self, const char *filepath)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        return NULL;
    }

    self->context->appendFile(self->context, filepath);

    NASetAdd(self->readingFileSet, (char *)filepath);

    yyscan_t scanner;
    MML_lex_init_extra(self, &scanner);
    YY_BUFFER_STATE state = MML__create_buffer(fp, YY_BUF_SIZE, scanner);
    MML__switch_to_buffer(state, scanner);

    Node *node = NULL;
    MML_parse(scanner, filepath, (void **)&node);

    MML__delete_buffer(state, scanner);
    MML_lex_destroy(scanner);
    fclose(fp);

    NASetRemove(self->readingFileSet, (char *)filepath);

    return node;
}

static Node *MMLParserParse(void *_self, const char *filepath)
{
    MMLParser *self = _self; 

    Node *node = MMLParserParseInternal(self, filepath);
    if (!node) {
        self->context->appendError(self->context, NULL, GeneralParseErrorFileNotFound, filepath, NULL);
    } 

    return node;
}

static void MMLParserDestroy(void *_self)
{
    MMLParser *self = _self;
    NASetDestroy(self->readingFileSet);
    NAMapDestroy(self->includedRootNodeMap);
    free(self);
}

DSLParser *MMLParserCreate(ParseContext *context)
{
    MMLParser *self = calloc(1, sizeof(MMLParser));
    self->context = context;
    self->parser.parse = MMLParserParse;
    self->parser.destroy = MMLParserDestroy;
    self->readingFileSet = NASetCreate(NAHashCString, NADescriptionCString);
    self->includedRootNodeMap = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    return (DSLParser *)self;
}

Node *MMLParserParseIncludeFile(void *_self, FileLocation *location, const char *includeFile, ASTInclude *includeNode)
{
    MMLParser *self = _self; 

    const char *ext = NAIOGetFileExtenssion(includeFile);
    if (0 != strcmp("mml", ext)) {
        self->context->appendError(self->context, location, MMLParseErrorUnsupportedFileTypeInclude, ext, includeFile, NULL);
        return NULL;
    }

    char *directory = dirname(location->filepath);
    char *fullpath = NAIOBuildPathWithDirectory(directory, includeFile);

    if (NASetContains(self->readingFileSet, fullpath)) {
        self->context->appendError(self->context, location, MMLParseErrorCircularFileInclude, includeFile, NULL);
        free(fullpath);
        return NULL;
    }

    Node *node = NAMapGet(self->includedRootNodeMap, fullpath);
    if (node) {
        free(fullpath);
        return NodeRetain(node);
    }

    node = MMLParserParseInternal(self, fullpath);
    if (!node) {
        self->context->appendError(self->context, location, MMLParseErrorIncludeFileNotFound, includeFile, NULL);
        free(fullpath);
        return NULL;
    }

    includeNode->fullpath = fullpath;
    NAMapPut(self->includedRootNodeMap, includeNode->fullpath, node);
    return node;
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
