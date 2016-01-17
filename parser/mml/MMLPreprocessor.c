#include "MMLPreprocessor.h"
#include "MML_preprocessor_lex.h"
#include "MMLParser.h"
#include "NAIO.h"
#include "NAStack.h"
#include "NASet.h"
#include "NAStringBuffer.h"
#include "NAMap.h"
#include "NAArray.h"
#include "NACString.h"
#include "NACInteger.h"
#include "NALog.h"

#include <stdlib.h>
#include <libgen.h>
#include <ctype.h>

#define LOCATION_FORMAT "##%s:%d:%d\n"

typedef struct Macro {
    char *target;
    char *replacement;
    NAMap *args;
    int targetLength;
} Macro;

static Macro *MacroCreate(char *target, char *replacement);
static void MacroDestroy(Macro *self);

typedef struct _Buffer {
    YY_BUFFER_STATE state;
    FILE *fp;
    char *filepath;
    int line;
    int column;
} Buffer;

static Buffer *BufferCreate(YY_BUFFER_STATE state, FILE *fp, char *filepath);
static void BufferDestroy(Buffer *self);
static int MacroLengthComparator(const void *p1, const void *p2);

struct _MMLPreprocessor {
    ParseContext *context;
    NASet *readingFileSet;

    yyscan_t scanner;
    Buffer *currentBuffer;
    NAStack *bufferStack;
    FILE *outputStream;

    NAMap *macros;
    NAArray *orderedMacroList;
    NASet *expandingMacroSet;
};

extern int MML_preprocessor_lex(yyscan_t yyscanner, FILE *stream);
extern void MML_preprocessor_set_column(int column_no, yyscan_t yyscanner);

MMLPreprocessor *MMLPreprocessorCreate(ParseContext *context)
{
    MMLPreprocessor *self = calloc(1, sizeof(MMLPreprocessor));
    self->context = context;
    self->readingFileSet = NASetCreate(NAHashCString, NADescriptionCString);
    self->bufferStack = NAStackCreate(4);
    self->outputStream = NAIOCreateMemoryStream(1024);
    self->macros = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionAddress);
    self->orderedMacroList = NAArrayCreate(4, NADescriptionAddress);
    self->expandingMacroSet = NASetCreate(NAHashAddress, NADescriptionAddress);
    return self;
}

void MMLPreprocessorDestroy(MMLPreprocessor *self)
{
    NAMapTraverseValue(self->macros, MacroDestroy);
    NAMapDestroy(self->macros);

    NAArrayDestroy(self->orderedMacroList);
    NASetDestroy(self->expandingMacroSet);

    NASetDestroy(self->readingFileSet);
    NAStackDestroy(self->bufferStack);
    free(self);
}

FILE *MMLPreprocessorScanFile(MMLPreprocessor *self, const char *filepath)
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

    fprintf(self->outputStream, LOCATION_FORMAT, self->currentBuffer->filepath, 1, 1);

    NASetAdd(self->readingFileSet, self->currentBuffer->filepath);

    MML_preprocessor_lex(self->scanner, self->outputStream);

    MML_preprocessor__delete_buffer(self->currentBuffer->state, self->scanner);
    MML_preprocessor_lex_destroy(self->scanner);
    fclose(self->currentBuffer->fp);

    NASetRemove(self->readingFileSet, self->currentBuffer->filepath);

    BufferDestroy(self->currentBuffer);

EXIT:
    free(fullpath);

    rewind(self->outputStream);

    if (__IsDebug__) {
        printf("\r-- preprocessed -----------\n");
        char str[1024];
        while (fgets(str, 1024, self->outputStream)) {
            fputs(str, stdout);
        }
        printf("---------------------------\n");
        rewind(self->outputStream);
    }

    return self->outputStream;
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

    fprintf(self->outputStream, LOCATION_FORMAT, self->currentBuffer->filepath, 1, 1);

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

    fprintf(self->outputStream, LOCATION_FORMAT, self->currentBuffer->filepath, self->currentBuffer->line, self->currentBuffer->column);

    MML_preprocessor__switch_to_buffer(self->currentBuffer->state, self->scanner);
    MML_preprocessor_set_lineno(self->currentBuffer->line, self->scanner);
    MML_preprocessor_set_column(self->currentBuffer->column, self->scanner);

    return true;
}

void MMLPreprocessorAppendMacro(MMLPreprocessor *self, int line, int column, char *difinition)
{
    FileLocation location = {self->currentBuffer->filepath, line, column};

    char **split = NACStringSplit(NACStringDuplicate(difinition), "=;", NULL);
    char *target = NACStringTrimWhiteSpace(split[0]);
    char *replacement = NACStringTrimWhiteSpace(split[1]);

    split = NACStringSplit(target, "{}", NULL);
    target = NACStringTrimWhiteSpace(split[0]);
    char *args = split[1];

    Macro *macro = NAMapRemove(self->macros, target);
    if (macro) {
        location.column = strstr(difinition, target) - difinition + 1;
        self->context->appendError(self->context, &location, MMLParseErrorMacroRedefined, target, NULL);

        int index = NAArrayFindFirstIndex(self->orderedMacroList, macro, NAArrayAddressComparator);
        NAArrayRemoveAt(self->orderedMacroList, index);
        MacroDestroy(macro);
    }

    macro = MacroCreate(target, replacement);
    if (args) {
        int argc;
        split = NACStringSplit(args, ", \t", &argc);
        for (int i = 0; i < argc; ++i) {
            if (NAMapContainsKey(macro->args, split[i])) {
                self->context->appendError(self->context, &location, MMLParseErrorDuplicatedMacroArguments, split[i], NULL);
                MacroDestroy(macro);
                return;
            }
            else {
                NAMapPut(macro->args, strdup(split[i]), NACIntegerFromInteger(i));
            }
        }
    }

    char *pc;
    while ((pc = strchr(replacement, '%'))) {
        int from = pc - replacement;
        int to = from + 1;
        char c;
        while ((c = replacement[to])) {
            if (isdigit(c) || isalpha(c) || '_' == c || '+' == c || '(' == c || ')' == c) {
                ++to;
            }
            else {
                break;
            }
        }

        char *replace = NACStringDuplicateNString(replacement + from, to - from);
        if (!NAMapContainsKey(macro->args, replace + 1)) {
            location.column = strstr(difinition, replace) - difinition + 1;
            self->context->appendError(self->context, &location, MMLParseErrorUndefinedMacroArgument, replace, NULL);
            MacroDestroy(macro);
            return;
        }

        replacement += to;
    }

    NAMapPut(self->macros, macro->target, macro);
    NAArrayAppend(self->orderedMacroList, macro);
    NAArraySort(self->orderedMacroList, MacroLengthComparator);
}

static void MMLPreprocessorExpandMacroInternal(MMLPreprocessor *self, FileLocation *location, NAStringBuffer *buffer, char *string)
{
    NAStringBuffer *expandBuffer = NAStringBufferCreate(256);

    char *pc;

    char *target = NACStringDuplicate(string);
    char *argsString = NULL;

    if ((pc = strchr(target, '{'))) {
        *pc = '\0';
        argsString = pc + 1;
        argsString[strlen(argsString) - 1] = '\0';
    }

    target = NACStringTrimWhiteSpace(target);

    char *remain = NULL;
    Macro *macro = NULL;
    NAIterator *iterator = NAArrayGetIterator(self->orderedMacroList);
    while (iterator->hasNext(iterator)) {
        Macro *_macro = iterator->next(iterator);
        char *p = strstr(target, _macro->target);
        if (p) {
            remain = p + _macro->targetLength;

            if (NASetContains(self->expandingMacroSet, _macro)) {
                self->context->appendError(self->context, location, MMLParseErrorCircularMacroReference, _macro->target, NULL);
                goto FINISH;
            }

            macro = _macro;
            break;
        }
    }
    
    if (!macro) {
        self->context->appendError(self->context, location, MMLParseErrorUndefinedMacroSymbol, target, NULL);
        remain = target + strlen(target);
        goto FINISH;
    }

    int macroArgc = NAMapCount(macro->args);
    if (0 == macroArgc && !argsString) {
        NAStringBufferAppendString(expandBuffer, macro->replacement);
        goto EXPANDED;
    }
    
    if (0 < macroArgc && !argsString) {
        self->context->appendError(self->context, location, MMLParseErrorMacroArgumentsMissing, NACStringFromInteger(macroArgc), NULL);
        goto FINISH;
    }

    int argc;
    char **args = NACStringSplit(argsString, ",", &argc);
    if (macroArgc != argc) {
        self->context->appendError(self->context, location, MMLParseErrorWrongNumberOfMacroArguments, NACStringFromInteger(argc), NACStringFromInteger(macroArgc), NULL);
        goto FINISH;
    }

    char *replacement = macro->replacement;
    while ((pc = strchr(replacement, '%'))) {
        int from = pc - replacement;
        int to = from + 1;
        char c;
        while ((c = replacement[to])) {
            if (isdigit(c) || isalpha(c) || '_' == c || '+' == c || '(' == c || ')' == c) {
                ++to;
            }
            else {
                break;
            }
        }

        NAStringBufferAppendNString(expandBuffer, replacement, from);

        char *replace = NACStringDuplicateNString(replacement + from, to - from);
        int *index = NAMapGet(macro->args, replace + 1);
        if (index) {
            NAStringBufferAppendString(expandBuffer, args[*index]);
        }

        replacement += to;
    }

    NAStringBufferAppendString(expandBuffer, replacement);

EXPANDED:
    ;

    char *expanded = NAStringBufferRetriveCString(expandBuffer);
    if ((pc = strchr(expanded, '$'))) {
        NAStringBufferAppendNString(buffer, expanded, pc - expanded);
        NASetAdd(self->expandingMacroSet, macro);
        MMLPreprocessorExpandMacroInternal(self, location, buffer, pc);
        NASetRemove(self->expandingMacroSet, macro);
    }
    else {
        NAStringBufferAppendString(buffer, expanded);
    }
    free(expanded);

FINISH:
    NAStringBufferAppendString(buffer, remain);
    NAStringBufferDestroy(expandBuffer);
}

char *MMLPreprocessorExpandMacro(MMLPreprocessor *self, int line, int column, char *string)
{
    FileLocation location = {self->currentBuffer->filepath, line, column};

    NAStringBuffer *buffer = NAStringBufferCreate(256);
    char *ret = NULL;

    MMLPreprocessorExpandMacroInternal(self, &location, buffer, string);
    
    if (0 < NAStringBufferGetLength(buffer)) {
        ret = NAStringBufferRetriveCString(buffer);
    }
    
    NAStringBufferDestroy(buffer);
    return ret;
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

static Macro *MacroCreate(char *target, char *replacement)
{
    Macro *self = calloc(1, sizeof(Macro));
    self->target = strdup(target);
    self->replacement = strdup(replacement);
    self->args = NAMapCreate(NAHashCString, NADescriptionCString, NADescriptionCInteger);
    self->targetLength = strlen(self->target);
    return self;
}

static void MacroDestroy(Macro *self)
{
    NAMapTraverseKey(self->args, free);
    NAMapTraverseValue(self->args, free);
    NAMapDestroy(self->args);

    free(self->target);
    free(self->replacement);
    free(self);
}

static int MacroLengthComparator(const void *p1, const void *p2)
{
    const Macro *macro1 = *((const Macro **)p1);
    const Macro *macro2 = *((const Macro **)p2);

    return macro2->targetLength - macro1->targetLength;
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
