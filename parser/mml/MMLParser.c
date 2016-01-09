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

typedef struct _Buffer {
    YY_BUFFER_STATE state;
    FILE *fp;
    char *filepath;
    int line;
    int column;
} Buffer;

static Buffer *BufferCreate(YY_BUFFER_STATE state, FILE *fp, char *filepath);
static void BufferDestroy(Buffer *self);

typedef struct _MMLParser {
    DSLParser parser;
    ParseContext *context;
    NASet *readingFileSet;

    yyscan_t scanner;
    Buffer *currentBuffer;
    NAStack *bufferStack;
} MMLParser;

extern int MML_parse(yyscan_t scanner, void **node);
extern void MML_set_column(int column_no, yyscan_t yyscanner);

static Node *MMLParserParseInternal(MMLParser *self, const char *filepath)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        return NULL;
    }

    MML_lex_init_extra(self, &self->scanner);
    YY_BUFFER_STATE state = MML__create_buffer(fp, YY_BUF_SIZE, self->scanner);
    MML__switch_to_buffer(state, self->scanner);

    char *_filepath = self->context->appendFile(self->context, filepath);
    self->currentBuffer = BufferCreate(state, fp, _filepath);

    NASetAdd(self->readingFileSet, self->currentBuffer->filepath);

    Node *node = NULL;
    MML_parse(self->scanner, (void **)&node);

    MML__delete_buffer(self->currentBuffer->state, self->scanner);
    MML_lex_destroy(self->scanner);
    fclose(self->currentBuffer->fp);

    NASetRemove(self->readingFileSet, self->currentBuffer->filepath);

    BufferDestroy(self->currentBuffer);

    return node;
}

static Node *MMLParserParse(void *_self, const char *filepath)
{
    MMLParser *self = _self; 

    MMLPreprocessor *preprocessor = MMLPreprocessorCreate(self->context);
    MMLPreprocessorScanFile(preprocessor, filepath);
    char *preprocessed;
    int length;
    MMLPreprocessorGetPreprocessedString(preprocessor, &preprocessed, &length);
    printf("---\n");
    fputs(preprocessed, stdout);
    printf("---\n");
    MMLPreprocessorDestroy(preprocessor);

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
    NAStackDestroy(self->bufferStack);
    free(self);
}

DSLParser *MMLParserCreate(ParseContext *context)
{
    MMLParser *self = calloc(1, sizeof(MMLParser));
    self->context = context;
    self->parser.parse = MMLParserParse;
    self->parser.destroy = MMLParserDestroy;
    self->readingFileSet = NASetCreate(NAHashCString, NADescriptionCString);
    self->bufferStack = NAStackCreate(4);
    return (DSLParser *)self;
}

void MMLParserParseIncludeFile(void *_self, int line, int column, const char *includeFile)
{
    MMLParser *self = _self; 

    FileLocation location = {self->currentBuffer->filepath, line, column};

    const char *ext = NAIOGetFileExtenssion(includeFile);
    if (0 != strcmp("mml", ext)) {
        self->context->appendError(self->context, &location, MMLParseErrorUnsupportedFileTypeInclude, ext, includeFile, NULL);
        return;
    }

    char *directory = dirname(location.filepath);
    char *fullpath = NAIOBuildPathWithDirectory(directory, includeFile);

    if (NASetContains(self->readingFileSet, fullpath)) {
        self->context->appendError(self->context, &location, MMLParseErrorCircularFileInclude, includeFile, NULL);
        goto EXIT;
    }

    FILE *fp = fopen(fullpath, "r");
    if (!fp) {
        self->context->appendError(self->context, &location, MMLParseErrorIncludeFileNotFound, includeFile, NULL);
        goto EXIT;
    }

    YY_BUFFER_STATE state = MML__create_buffer(fp, YY_BUF_SIZE, self->scanner);
    MML__switch_to_buffer(state, self->scanner);
    MML_set_lineno(1, self->scanner);
    MML_set_column(1, self->scanner);

    self->currentBuffer->line = line;
    self->currentBuffer->column = column;
    NAStackPush(self->bufferStack, self->currentBuffer);

    char *_filepath = self->context->appendFile(self->context, fullpath);
    self->currentBuffer = BufferCreate(state, fp, _filepath);

    NASetAdd(self->readingFileSet, self->currentBuffer->filepath);

EXIT:
    free(fullpath);
}

bool MMLParserPopPreviousFile(void *_self)
{
    MMLParser *self = _self;

    if (0 == NAStackCount(self->bufferStack)) {
        return false;
    }

    MML__delete_buffer(self->currentBuffer->state, self->scanner);
    fclose(self->currentBuffer->fp);
    BufferDestroy(self->currentBuffer);

    self->currentBuffer = NAStackPop(self->bufferStack);
    MML__switch_to_buffer(self->currentBuffer->state, self->scanner);
    MML_set_lineno(self->currentBuffer->line, self->scanner);
    MML_set_column(self->currentBuffer->column, self->scanner);

    return true;
}

char *MMLParserGetCurrentFilepath(void *_self)
{
    MMLParser *self = _self;
    return self->currentBuffer->filepath;
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

static Buffer *BufferCreate(YY_BUFFER_STATE state, FILE *fp, char *filepath)
{
    Buffer *self = calloc(1, sizeof(Buffer));
    self->state = state;
    self->fp = fp;
    self->filepath = filepath;
    return self;
}

static void BufferDestroy(Buffer *self)
{
    free(self);
}
