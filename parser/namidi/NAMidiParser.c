#include "NAMidiParser.h"
#include "NAMidiAST.h"
#include "NAIO.h"
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"
#include "NASet.h"
#include "NAMap.h"

#include <stdlib.h>
#include <libgen.h>

typedef struct _NAMidiParser {
    DSLParser parser;
    ParseContext *context;
    NASet *readingFileSet;
    NAMap *includedRootNodeMap;
} NAMidiParser;

extern int NAMidi_parse(yyscan_t scanner, const char *filepath, void **node);

static Node *NAMidiParserParseInternal(NAMidiParser *self, const char *filepath)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        return NULL;
    }

    self->context->appendFile(self->context, filepath);

    NASetAdd(self->readingFileSet, (char *)filepath);

    yyscan_t scanner;
    NAMidi_lex_init_extra(self, &scanner);
    YY_BUFFER_STATE state = NAMidi__create_buffer(fp, YY_BUF_SIZE, scanner);
    NAMidi__switch_to_buffer(state, scanner);

    Node *node = NULL;
    NAMidi_parse(scanner, filepath, (void **)&node);

    NAMidi__delete_buffer(state, scanner);
    NAMidi_lex_destroy(scanner);
    fclose(fp);

    NASetRemove(self->readingFileSet, (char *)filepath);

    return node;
}

static Node *NAMidiParserParse(void *_self, const char *filepath)
{
    NAMidiParser *self = _self; 

    Node *node = NAMidiParserParseInternal(self, filepath);
    if (!node) {
        self->context->appendError(self->context, NULL, GeneralParseErrorFileNotFound, filepath, NULL);
    } 

    return node;
}

static void NAMidiParserDestroy(void *_self)
{
    NAMidiParser *self = _self;
    NASetDestroy(self->readingFileSet);
    NAMapDestroy(self->includedRootNodeMap);
    free(self);
}

DSLParser *NAMidiParserCreate(ParseContext *context)
{
    NAMidiParser *self = calloc(1, sizeof(NAMidiParser));
    self->context = context;
    self->parser.parse = NAMidiParserParse;
    self->parser.destroy = NAMidiParserDestroy;
    self->readingFileSet = NASetCreate(NAHashCString, NADescriptionCString);
    self->includedRootNodeMap = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    return (DSLParser *)self;
}

Node *NAMidiParserParseIncludeFile(void *_self, FileLocation *location, const char *includeFile, ASTInclude *includeNode)
{
    NAMidiParser *self = _self; 

    const char *ext = NAIOGetFileExtenssion(includeFile);
    if (0 != strcmp("namidi", ext) && 0 != strcmp("nas", ext)) {
        self->context->appendError(self->context, location, NAMidiParseErrorUnsupportedFileTypeInclude, ext, includeFile, NULL);
        return NULL;
    }

    char *directory = dirname(location->filepath);
    char *fullpath = NAIOBuildPathWithDirectory(directory, includeFile);

    if (NASetContains(self->readingFileSet, fullpath)) {
        self->context->appendError(self->context, location, NAMidiParseErrorCircularFileInclude, includeFile, NULL);
        free(fullpath);
        return NULL;
    }

    Node *node = NAMapGet(self->includedRootNodeMap, fullpath);
    if (node) {
        free(fullpath);
        return NodeRetain(node);
    }

    node = NAMidiParserParseInternal(self, fullpath);
    if (!node) {
        self->context->appendError(self->context, location, NAMidiParseErrorIncludeFileNotFound, includeFile, NULL);
        free(fullpath);
        return NULL;
    }

    includeNode->fullpath = fullpath;
    NAMapPut(self->includedRootNodeMap, includeNode->fullpath, node);
    return node;
}

void NAMidiParserSyntaxError(void *_self, FileLocation *location, const char *token)
{
    NAMidiParser *self = _self; 
    self->context->appendError(self->context, location, GeneralParseErrorSyntaxError, token, NULL);
}
