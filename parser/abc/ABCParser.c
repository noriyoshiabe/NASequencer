#include "ABCParser.h"
#include "ABCAST.h"
#include "NAUtil.h"
#include "ABC_yacc.h"
#include "ABC_lex.h"
#include "ABC_information_yacc.h"
#include "ABC_information_lex.h"
#include "NASet.h"
#include "NAMap.h"
#include "NACString.h"

#include <stdlib.h>
#include <libgen.h>
#include <regex.h>

typedef struct Macro {
    char *target;
    char *replacement;
    regex_t reg;
    bool transposing;
} Macro;

typedef struct _ABCParser {
    DSLParser parser;
    ParseContext *context;
    NASet *readingFileSet;
    NAMap *includedNodeMap;
    char lineBreak;
    char decorationDialects[2];
    NAMap *staticMacros;
    NAMap *transposingMacros;
} ABCParser;

extern int ABC_parse(yyscan_t scanner, const char *filepath, void **node);
extern int ABC_information_parse(yyscan_t scanner, const char *filepath, int line, void **node);

static Macro *MacroCreate(char *target, char *replacement);
static void MacroDestroy(Macro *self);

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

    NAMapTraverseValue(self->staticMacros, MacroDestroy);
    NAMapDestroy(self->staticMacros);

    NAMapTraverseValue(self->transposingMacros, MacroDestroy);
    NAMapDestroy(self->transposingMacros);

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
    self->lineBreak = '\n';
    self->decorationDialects[0] = '!';
    self->decorationDialects[1] = '+';
    self->staticMacros = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    self->transposingMacros = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);

    return (DSLParser *)self;
}

Node *ABCParserParseInformation(void *_self, const char *filepath, int line, const char *string)
{
    ABCParser *self = _self; 
    Node *node = NULL;

    yyscan_t informationScanner;
    ABC_information_lex_init_extra(self, &informationScanner);
    YY_BUFFER_STATE state = ABC_information__scan_string(string, informationScanner);
    ABC_information_parse(informationScanner, filepath, line, (void **)&node);
    ABC_information__delete_buffer(state, informationScanner);
    ABC_information_lex_destroy(informationScanner);

    return node;
}

Node *ABCParserParseIncludeFile(void *_self, FileLocation *location, const char *includeFile, char **_fullpath)
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

    *_fullpath = fullpath;
    NAMapPut(self->includedNodeMap, fullpath, node);
    return node;
}

void ABCParserSyntaxError(void *_self, FileLocation *location, const char *token)
{
    ABCParser *self = _self; 
    self->context->appendError(self->context, location, GeneralParseErrorSyntaxError, token, NULL);
}

void ABCParserSetLineBreak(void *_self, char c)
{
    ABCParser *self = _self; 
    self->lineBreak = c;

    if ('!' == c) {
        self->decorationDialects[0] = '+';
        self->decorationDialects[1] = -1;
    }
}

bool ABCParserIsLineBreak(void *_self, char c)
{
    ABCParser *self = _self; 
    return self->lineBreak == c;
}

void ABCParserSetDecoration(void *_self, char c)
{
    ABCParser *self = _self;
    self->decorationDialects[0] = c;
    self->decorationDialects[1] = -1;

    if (c == self->lineBreak) {
        self->lineBreak = -1;
    }
}

bool ABCParserIsDecoration(void *_self, char c)
{
    ABCParser *self = _self; 
    return self->decorationDialects[0] == c || self->decorationDialects[1] == c;
}

void ABCParserSetMacro(void *_self, char *target, char *replacement)
{
    ABCParser *self = _self;

    Macro *prev;
    if ((prev = NAMapRemove(self->staticMacros, target))) {
        MacroDestroy(prev);
    }
    if ((prev = NAMapRemove(self->transposingMacros, target))) {
        MacroDestroy(prev);
    }
    
    Macro *macro = MacroCreate(target, replacement);

    if (macro->transposing) {
        NAMapPut(self->transposingMacros, macro->target, macro);
    }
    else {
        NAMapPut(self->staticMacros, macro->target, macro);
    }    
}


static Macro *MacroCreate(char *target, char *replacement)
{
    Macro *self = calloc(1, sizeof(Macro));
    self->target = target;
    self->replacement = NACStringTrimWhiteSpace(replacement);

    char *pattern = NACStringDuplicate(target);
    char *n = strchr(pattern, 'n');
    if (n) {
        char *_pattern = pattern;
        pattern = alloca(strlen(_pattern) + 10 + 1);
        *n = '\0';
        sprintf(pattern, "%s([A-Ga-g])%s", _pattern, n + 1);
        self->transposing = true;
    }
    regcomp(&self->reg, pattern, REG_EXTENDED);

    return self;
}

static void MacroDestroy(Macro *self)
{
    free(self->target);
    free(self->replacement);
    free(self);
}
