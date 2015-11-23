#include "NAMidiParser.h"
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"
#include "NAMidiExpression.h"

#include "NAUtil.h"
#include "NASet.h"
#include "NACString.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <libgen.h>
#include <sys/param.h>

extern int NAMidi_parse(yyscan_t scanner, const char *filepath, Expression **expression);

struct _NAMidiParser {
    DSLParser interface;
    ParserCallbacks *callbacks;
    void *receiver;
    SequenceBuilder *builder;

    NASet *fileSet;
    NASet *readingFileSet;
};

static bool NAMidiParserParseFileInternal(NAMidiParser *self, const char *filepath, Expression **expression);
static void NAMidiParserBuildPattenMap1(NAMidiParser *self, Expression *expression, NAMap *patternMap);
static void NAMidiParserBuildPattenMap2(NAMidiParser *self, Expression *expression, NAMap *patternMap);

static bool NAMidiParserParseFile(void *_self, const char *filepath)
{
    NAMidiParser *self = _self;

    Expression *expression;

    if (!NAMidiParserParseFileInternal(self, filepath, &expression)) {
        NAMidiParserError(self, NULL, GeneralParseErrorFileNotFound, filepath, NULL);
        return false;
    }

    NAMidiParserBuildPattenMap1(self, expression, NULL);
    NAMidiParserBuildPattenMap2(self, expression, NULL);

    bool success = ExpressionParse(expression, self, NULL);
    ExpressionDestroy(expression);
    return success;
}

bool NAMidiParserReadIncludeFile(NAMidiParser *self, ParseLocation *location, char *includeFile, Expression **expression)
{
    const char *ext = NAUtilGetFileExtenssion(includeFile);
    if (0 != strcmp("namidi", ext)) {
        NAMidiParserError(self, location, NAMidiParseErrorUnsupportedFileTypeInclude, ext, includeFile, NULL);
        free(includeFile);
        return false;
    }

    char *directory = dirname((char *)location->filepath);
    char *fullPath = NAUtilBuildPathWithDirectory(directory, includeFile);

    free(includeFile);

    if (NASetContains(self->readingFileSet, fullPath)) {
        NAMidiParserError(self, location, NAMidiParseErrorCircularFileInclude, includeFile, NULL);
        free(fullPath);
        return false;
    }

    bool success = NAMidiParserParseFileInternal(self, fullPath, expression);
    if (!success) {
        NAMidiParserError(self, location, NAMidiParseErrorIncludeFileNotFound, includeFile, NULL);
    }

    free(fullPath);
    return success;
}

static bool NAMidiParserParseFileInternal(NAMidiParser *self, const char *filepath, Expression **expression)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        return false;
    }

    char *copiedFilePath;
    if (!(copiedFilePath = NASetGet(self->fileSet, (char *)filepath))) {
        copiedFilePath = strdup(filepath);
        NASetAdd(self->fileSet, copiedFilePath);
        self->callbacks->onReadFile(self->receiver, copiedFilePath);
    }

    NASetAdd(self->readingFileSet, copiedFilePath);

    yyscan_t scanner;
    NAMidi_lex_init_extra(self, &scanner);
    YY_BUFFER_STATE state = NAMidi__create_buffer(fp, YY_BUF_SIZE, scanner);
    NAMidi__switch_to_buffer(state, scanner);

    bool success = 0 == NAMidi_parse(scanner, copiedFilePath, expression);

    NASetRemove(self->readingFileSet, copiedFilePath);
    
    NAMidi__delete_buffer(state, scanner);
    NAMidi_lex_destroy(scanner);
    fclose(fp);

    return success;
}

static void NAMidiParserBuildPattenMap1(NAMidiParser *self, Expression *expression, NAMap *patternMap)
{
    if (NAMidiExprIsStatementList(expression)) {
        patternMap = NAMidiExprStatementListGetPatternMap(expression);
    }

    if (NAMidiExprIsPattern(expression)) {
        char *identifier = NAMidiExprPatternGetIdentifier(expression);
        if (NAMapContainsKey(patternMap, identifier)) {
            Expression *original = NAMapGet(patternMap, identifier);
            NAMidiParserError(self, &expression->location, NAMidiParseErrorDuplicatePatternIdentifier,
                    identifier, original->location.filepath, NACStringFromInteger(original->location.line), NULL);
            return;
        }
        else {
            NAMapPut(patternMap, identifier, expression);
        }
    }

    if (expression->children) {
        NAIterator *iterator = NAArrayGetIterator(expression->children);
        while (iterator->hasNext(iterator)) {
            NAMidiParserBuildPattenMap1(self, iterator->next(iterator), patternMap);
        }
    }
}

static void NAMidiParserBuildPattenMap2(NAMidiParser *self, Expression *expression, NAMap *patternMap)
{
    if (NAMidiExprIsStatementList(expression)) {
        NAMap *_patternMap = NAMidiExprStatementListGetPatternMap(expression);

        if (patternMap) {
            NAIterator *iterator = NAMapGetIterator(patternMap);

            while (iterator->hasNext(iterator)) {
                NAMapEntry *entry = iterator->next(iterator);
                NAMapPut(_patternMap, entry->key, entry->value);
            }
        }

        patternMap = _patternMap;
    }

    if (expression->children) {
        NAIterator *iterator = NAArrayGetIterator(expression->children);
        while (iterator->hasNext(iterator)) {
            NAMidiParserBuildPattenMap2(self, iterator->next(iterator), patternMap);
        }
    }
}

int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, Expression **expression, const char *message)
{
    ParseLocation location = {(char *)filepath, yylloc->first_line, yylloc->first_column};
    NAMidiParserError(NAMidi_get_extra(scanner), &location, GeneralParseErrorSyntaxError, NAMidi_get_text(scanner), NULL);
    return 0;
}

void NAMidiParserError(NAMidiParser *self, ParseLocation *location, int code, ...)
{
    va_list argList;
    va_start(argList, code);
    ParseError *error = ParseErrorCreateWithArgs(location, code, argList);
    va_end(argList);

    self->callbacks->onParseError(self->receiver, error);
}

SequenceBuilder *NAMidiParserGetBuilder(NAMidiParser *self)
{
    return self->builder;
}

static void NAMidiParserDestroy(void *_self)
{
    NAMidiParser *self = _self;
    NASetTraverse(self->fileSet, free);
    NASetDestroy(self->fileSet);
    NASetDestroy(self->readingFileSet);
    free(self);
}

DSLParser *NAMidiParserCreate(SequenceBuilder *builder, ParserCallbacks *callbacks, void *receiver)
{
    NAMidiParser *self = calloc(1, sizeof(NAMidiParser));
    self->interface.parseFile = NAMidiParserParseFile;
    self->interface.destroy = NAMidiParserDestroy;
    self->builder = builder;
    self->callbacks = callbacks;
    self->receiver = receiver;
    self->fileSet = NASetCreate(NAHashCString, NADescriptionCString);
    self->readingFileSet = NASetCreate(NAHashCString, NADescriptionCString);
    return (DSLParser *)self;
}

NAMidiParserContext *NAMidiParserContextCreate()
{
    NAMidiParserContext *self = calloc(1, sizeof(NAMidiParserContext));

    self->noteTable = NoteTableCreate(BaseNote_C, false, false, ModeMajor);

    for (int i = 0; i < 16; ++i) {
        self->channels[i].gatetime = 240;
        self->channels[i].velocity = 100;
        self->channels[i].octave = 2;
    }

    self->channel = 1;
    self->contextIdList = NASetCreate(NAHashCString, NADescriptionCString);
    self->expandingPatternList = NASetCreate(NAHashAddress, NADescriptionAddress);

    return self;
}

NAMidiParserContext *NAMidiParserContextCreateCopy(NAMidiParserContext *self)
{
    NAMidiParserContext *copy = calloc(1, sizeof(NAMidiParserContext));

    memcpy(copy, self, sizeof(NAMidiParserContext));
    copy->contextIdList = NASetCreate(NAHashCString, NADescriptionCString);

    NoteTableRetain(copy->noteTable);

    NAIterator *iterator = NASetGetIterator(self->contextIdList);
    while (iterator->hasNext(iterator)) {
        NASetAdd(copy->contextIdList, iterator->next(iterator));
    }

    copy->copy = true;
    return copy;
}

void NAMidiParserContextDestroy(NAMidiParserContext *self)
{
    NoteTableRelease(self->noteTable);

    if (!self->copy) {
        NASetDestroy(self->expandingPatternList);
    }

    NASetDestroy(self->contextIdList);
    free(self);
}

int NAMidiParserContextGetLength(NAMidiParserContext *self)
{
    int length = 0;
    for (int i = 0; i < 16; ++i) {
        length = MAX(length, self->channels[i].tick);
    }
    return length;
}
