#include "MMLPreprocessor.h"
#include "MML_preprocessor_lex.h"
#include "MMLParser.h"
#include "NAIO.h"
#include "NAStack.h"
#include "NASet.h"
#include "NAStringBuffer.h"

#include <stdlib.h>
#include <libgen.h>

#define LOCATION_FORMAT "##%s:%d:%d\n"

typedef struct _Buffer {
    YY_BUFFER_STATE state;
    FILE *fp;
    char *filepath;
    int line;
    int column;
} Buffer;

static Buffer *BufferCreate(YY_BUFFER_STATE state, FILE *fp, char *filepath);
static void BufferDestroy(Buffer *self);

struct _MMLPreprocessor {
    ParseContext *context;
    NASet *readingFileSet;

    yyscan_t scanner;
    Buffer *currentBuffer;
    NAStack *bufferStack;
    NAStringBuffer *stringBuffer;
};

extern int MML_preprocessor_lex(yyscan_t yyscanner, void *buffer);
extern void MML_preprocessor_set_column(int column_no, yyscan_t yyscanner);

MMLPreprocessor *MMLPreprocessorCreate(ParseContext *context)
{
    MMLPreprocessor *self = calloc(1, sizeof(MMLPreprocessor));
    self->context = context;
    self->readingFileSet = NASetCreate(NAHashCString, NADescriptionCString);
    self->bufferStack = NAStackCreate(4);
    self->stringBuffer = NAStringBufferCreate(1024);
    return self;
}

void MMLPreprocessorDestroy(MMLPreprocessor *self)
{
    NASetDestroy(self->readingFileSet);
    NAStackDestroy(self->bufferStack);
    NAStringBufferDestroy(self->stringBuffer);
    free(self);
}

void MMLPreprocessorScanFile(MMLPreprocessor *self, const char *filepath)
{
    char *fullpath = NAIOGetRealPath(filepath);
    FILE *fp = fopen(fullpath, "r");
    if (!fp) {
        self->context->appendError(self->context, NULL, GeneralParseErrorFileNotFound, filepath, NULL);
        goto EXIT;
    }

    MML_preprocessor_lex_init_extra(self, &self->scanner);
    YY_BUFFER_STATE state = MML_preprocessor__create_buffer(fp, YY_BUF_SIZE, self->scanner);
    MML_preprocessor__switch_to_buffer(state, self->scanner);

    char *_filepath = self->context->appendFile(self->context, filepath);
    self->currentBuffer = BufferCreate(state, fp, _filepath);

    NAStringBufferAppendFormat(self->stringBuffer, LOCATION_FORMAT, self->currentBuffer->filepath, 1, 1);

    NASetAdd(self->readingFileSet, self->currentBuffer->filepath);

    MML_preprocessor_lex(self->scanner, self->stringBuffer);

    MML_preprocessor__delete_buffer(self->currentBuffer->state, self->scanner);
    MML_preprocessor_lex_destroy(self->scanner);
    fclose(self->currentBuffer->fp);

    NASetRemove(self->readingFileSet, self->currentBuffer->filepath);

    BufferDestroy(self->currentBuffer);

EXIT:
    free(fullpath);
}

void MMLPreprocessorGetPreprocessedString(MMLPreprocessor *self, char **string, int *length)
{
    *length = NAStringBufferGetLength(self->stringBuffer);
    *string = NAStringBufferRetriveCString(self->stringBuffer);
}

void MMLPreprocessorIncludeFile(MMLPreprocessor *self, int line, int column, const char *includeFile)
{
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

    YY_BUFFER_STATE state = MML_preprocessor__create_buffer(fp, YY_BUF_SIZE, self->scanner);
    MML_preprocessor__switch_to_buffer(state, self->scanner);
    MML_preprocessor_set_lineno(1, self->scanner);
    MML_preprocessor_set_column(1, self->scanner);

    self->currentBuffer->line = line;
    self->currentBuffer->column = column;
    NAStackPush(self->bufferStack, self->currentBuffer);

    char *_filepath = self->context->appendFile(self->context, fullpath);
    self->currentBuffer = BufferCreate(state, fp, _filepath);

    NAStringBufferAppendFormat(self->stringBuffer, LOCATION_FORMAT, self->currentBuffer->filepath, 1, 1);

    NASetAdd(self->readingFileSet, self->currentBuffer->filepath);

EXIT:
    free(fullpath);
}

bool MMLPreprocessorPopPreviousFile(MMLPreprocessor *self)
{
    if (0 == NAStackCount(self->bufferStack)) {
        return false;
    }

    MML_preprocessor__delete_buffer(self->currentBuffer->state, self->scanner);
    fclose(self->currentBuffer->fp);
    BufferDestroy(self->currentBuffer);

    self->currentBuffer = NAStackPop(self->bufferStack);

    NAStringBufferAppendFormat(self->stringBuffer, LOCATION_FORMAT, self->currentBuffer->filepath, self->currentBuffer->line, self->currentBuffer->column);

    MML_preprocessor__switch_to_buffer(self->currentBuffer->state, self->scanner);
    MML_preprocessor_set_lineno(self->currentBuffer->line, self->scanner);
    MML_preprocessor_set_column(self->currentBuffer->column, self->scanner);

    return true;
}

void MMLPreprocessorAppendMacro(MMLPreprocessor *self, int line, int column, char *difinition)
{
}

char *MMLPreprocessorExpandMacro(MMLPreprocessor *self, int line, int column, char *string)
{
    return NULL;
}

void MMLPreprocessorSyntaxError(MMLPreprocessor *self, int line, int column, const char *token)
{
    FileLocation location = {self->currentBuffer->filepath, line, column};
    self->context->appendError(self->context, &location, GeneralParseErrorSyntaxError, token, NULL);
}

void MMLPreprocessorUnexpectedEOF(MMLPreprocessor *self, int line, int column)
{
    FileLocation location = {self->currentBuffer->filepath, line, column};
    self->context->appendError(self->context, &location, MMLParseErrorUnexpectedEOF, NULL);
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
