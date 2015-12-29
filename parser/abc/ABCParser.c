#include "ABCParser.h"
#include "ABCPreprocessor.h"
#include "ABCAST.h"
#include "NAUtil.h"
#include "ABC_yacc.h"
#include "ABC_lex.h"
#include "ABC_information_lex.h"
#include "ABC_directive_lex.h"
#include "ABC_tune_body_lex.h"
#include "NASet.h"
#include "NAMap.h"

#include <stdlib.h>
#include <libgen.h>

typedef struct {
    char *first;
    char *second;
} StringPair;

typedef struct _ABCParser {
    DSLParser parser;
    ParseContext *context;
    NASet *readingFileSet;
    NAMap *includedNodeMap;
    char lineBreak;
    char decorationDialects[2];
    ABCPreprocessor *preprocessor;
    bool inFileHeader;

    struct {
        char lineBreak;
        char decoration;
        NAArray *macros;
        NAArray *redefinableSymbols;
    } defaults;
} ABCParser;

extern int ABC_parse(yyscan_t scanner, const char *filepath, void **node);
extern int ABC_information_parse(yyscan_t scanner, const char *filepath, int line, int columnOffset, void **node);
extern int ABC_directive_parse(yyscan_t scanner, const char *filepath, int line, int columnOffset, void **node);
extern int ABC_tune_body_parse(yyscan_t scanner, const char *filepath, int line, int columnOffset, void **node);

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

    ABCPreprocessorDestroy(self->preprocessor);
    NAArrayDestroy(self->defaults.macros);
    NAArrayDestroy(self->defaults.redefinableSymbols);

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

    self->preprocessor = ABCPreprocessorCreate();
    self->inFileHeader = true;

    self->defaults.lineBreak = -1;
    self->defaults.decoration = -1;
    self->defaults.macros = NAArrayCreate(4, NULL);
    self->defaults.redefinableSymbols = NAArrayCreate(4, NULL);

    return (DSLParser *)self;
}

Node *ABCParserParseInformation(void *_self, const char *filepath, int line, int columnOffset, const char *string)
{
    ABCParser *self = _self; 
    Node *node = NULL;

    yyscan_t scanner;
    ABC_information_lex_init_extra(self, &scanner);
    YY_BUFFER_STATE state = ABC_information__scan_string(string, scanner);
    ABC_information_parse(scanner, filepath, line, columnOffset, (void **)&node);
    ABC_information__delete_buffer(state, scanner);
    ABC_information_lex_destroy(scanner);

    return node;
}

Node *ABCParserParseDirective(void *_self, const char *filepath, int line, int columnOffset, const char *string)
{
    ABCParser *self = _self; 
    Node *node = NULL;

    yyscan_t scanner;
    ABC_directive_lex_init_extra(self, &scanner);
    YY_BUFFER_STATE state = ABC_directive__scan_string(string, scanner);
    ABC_directive_parse(scanner, filepath, line, columnOffset, (void **)&node);
    ABC_directive__delete_buffer(state, scanner);
    ABC_directive_lex_destroy(scanner);

    return node;
}

Node *ABCParserParseTuneBody(void *_self, const char *filepath, int line, int columnOffset, const char *string)
{
    ABCParser *self = _self; 
    Node *node = NULL;

    char *preprocessed = ABCPreprocessorPreprocessTuneBody(self->preprocessor, string);

# if 0
    printf("== [tune body] ===========================================\n");
    printf("%s\n", string);
    printf("== [preprocessed] ========================================\n");
    printf("%s\n", preprocessed);
    printf("==========================================================\n");
#endif

    yyscan_t scanner;
    ABC_tune_body_lex_init_extra(self, &scanner);
    YY_BUFFER_STATE state = ABC_tune_body__scan_string(preprocessed, scanner);
    ABC_tune_body_parse(scanner, filepath, line, columnOffset, (void **)&node);
    ABC_tune_body__delete_buffer(state, scanner);
    ABC_tune_body_lex_destroy(scanner);

    free(preprocessed);

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

static void ABCParserSetLineBreak(void *_self, char c)
{
    ABCParser *self = _self; 
    self->lineBreak = c;

    if ('!' == c) {
        self->decorationDialects[0] = '+';
        self->decorationDialects[1] = -1;

        ABCPreprocessorSetDecorationDialect(self->preprocessor, '+');
    }
}

bool ABCParserIsLineBreak(void *_self, char c)
{
    ABCParser *self = _self; 
    return self->lineBreak == c;
}

static void ABCParserSetDecoration(void *_self, char c)
{
    ABCParser *self = _self;
    self->decorationDialects[0] = c;
    self->decorationDialects[1] = -1;

    if (c == self->lineBreak) {
        self->lineBreak = -1;
    }

    ABCPreprocessorSetDecorationDialect(self->preprocessor, c);
}

bool ABCParserIsDecoration(void *_self, char c)
{
    ABCParser *self = _self; 
    return self->decorationDialects[0] == c || self->decorationDialects[1] == c;
}

void ABCParserNotify(void *_self, ABCParserEvent event, void *node)
{
    ABCParser *self = _self;

    switch (event) {
    case ABCParserEventRefernceNumber:
        {
            NAIterator *iterator;

            self->inFileHeader = false;

            ABCPreprocessorDestroy(self->preprocessor);
            self->preprocessor = ABCPreprocessorCreate();

            self->lineBreak = '\n';
            self->decorationDialects[0] = '!';
            self->decorationDialects[1] = '+';

            if (-1 != self->defaults.lineBreak) {
                ABCParserSetLineBreak(self, self->defaults.lineBreak);
            }

            if (-1 == self->defaults.decoration) {
                ABCParserSetDecoration(self, self->defaults.decoration);
            }

            iterator = NAArrayGetIterator(self->defaults.macros);
            while (iterator->hasNext(iterator)) {
                ASTMacro *macro = iterator->next(iterator);
                ABCPreprocessorSetMacro(self->preprocessor, macro->target, macro->replacement);
            }

            iterator = NAArrayGetIterator(self->defaults.redefinableSymbols);
            while (iterator->hasNext(iterator)) {
                ASTRedefinableSymbol *rdSymbol = iterator->next(iterator);
                ABCPreprocessorSetMacro(self->preprocessor, rdSymbol->symbol, rdSymbol->replacement);
            }
        }
        break;
    case ABCParserEventInstLineBreak:
        {
            ASTInstLineBreak *ast = node;
            ABCParserSetLineBreak(self, ast->character);

            if (self->inFileHeader) {
                self->defaults.lineBreak = ast->character;
            }
        }
        break;
    case ABCParserEventInstDecoration:
        {
            ASTInstDecoration *ast = node;
            ABCParserSetDecoration(self, ast->character);

            if (self->inFileHeader) {
                self->defaults.decoration = ast->character;
            }
        }
        break;
    case ABCParserEventMacro:
        {
            ASTMacro *macro = node;
            ABCPreprocessorSetMacro(self->preprocessor, macro->target, macro->replacement);

            if (self->inFileHeader) {
                NAArrayAppend(self->defaults.macros, node);
            }
        }
        break;
    case ABCParserEventRedefinableSymbol:
        {
            ASTRedefinableSymbol *rdSymbol = node;
            ABCPreprocessorSetRedefinableSymbol(self->preprocessor, rdSymbol->symbol, rdSymbol->replacement);

            if (self->inFileHeader) {
                NAArrayAppend(self->defaults.redefinableSymbols, node);
            }
        }
        break;
    }
}
