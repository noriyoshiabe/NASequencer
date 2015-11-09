#include "NAMidiParser.h"
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"
#include "NAMidiExpression.h"

#include "NAUtil.h"
#include "NASet.h"

#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/param.h>

extern int NAMidi_parse(yyscan_t scanner, const char *filepath, Expression **expression);

struct _NAMidiParser {
    Parser interface;
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
        return false;
    }

    NAMidiParserBuildPattenMap1(self, expression, NULL);
    NAMidiParserBuildPattenMap2(self, expression, NULL);

    if (!ExpressionParse(expression, self, NULL)) {
        ExpressionDestroy(expression);
        return false;
    }

    return true;
}

bool NAMidiParserReadIncludeFile(NAMidiParser *self, ParseLocation *location, char *includeFile, Expression **expression)
{
    if (0 != strcmp("namidi", NAUtilGetFileExtenssion(includeFile))) {
        NAMidiParserError(self, location, ParseErrorKindGeneral, GeneralParseErrorUnsupportedFileType);
        free(includeFile);
        return false;
    }

    char *directory = dirname((char *)location->filepath);
    char *fullPath = NAUtilBuildPathWithDirectory(directory, includeFile);

    free(includeFile);

    if (NASetContains(self->readingFileSet, fullPath)) {
        NAMidiParserError(self, location, ParseErrorKindGeneral, GeneralParseErrorCircularFileInclude);
        free(fullPath);
        return false;
    }

    bool success = NAMidiParserParseFileInternal(self, fullPath, expression);
    free(fullPath);
    return success;
}

static bool NAMidiParserParseFileInternal(NAMidiParser *self, const char *filepath, Expression **expression)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        NAMidiParserError(self, &(ParseLocation){filepath, 0, 0}, ParseErrorKindGeneral, GeneralParseErrorFileNotFound);
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
        NAMapPut(patternMap, NAMidiExprPatternGetIdentifier(expression), expression);
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
            uint8_t iteratorBuffer[NAMapIteratorSize];
            NAIterator *iterator = NAMapGetIterator(patternMap, iteratorBuffer);

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
    NAMidiParserError(NAMidi_get_extra(scanner), &(ParseLocation){filepath, yylloc->first_line, yylloc->first_column}, ParseErrorKindGeneral, GeneralParseErrorSyntaxError);
    return 0;
}

void NAMidiParserError(NAMidiParser *self, ParseLocation *location, ParseErrorKind kind, int error)
{
    ParseError err = { .location = *location, .kind = kind, .error = error };
    self->callbacks->onParseError(self->receiver, &err);
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

Parser *NAMidiParserCreate(SequenceBuilder *builder, ParserCallbacks *callbacks, void *receiver)
{
    NAMidiParser *self = calloc(1, sizeof(NAMidiParser));
    self->interface.parseFile = NAMidiParserParseFile;
    self->interface.destroy = NAMidiParserDestroy;
    self->callbacks = callbacks;
    self->receiver = receiver;
    self->builder = builder;
    self->fileSet = NASetCreate(NAHashCString, NADescriptionCString);
    self->readingFileSet = NASetCreate(NAHashCString, NADescriptionCString);
    return (Parser *)self;
}

NAMidiParserContext *NAMidiParserContextCreate()
{
    NAMidiParserContext *self = calloc(1, sizeof(NAMidiParserContext));

    for (int i = 0; i < 16; ++i) {
        self->channels[i].gatetime = 240;
        self->channels[i].velocity = 100;
        self->channels[i].octave = 2;
    }

    self->contextIdList = NASetCreate(NAHashCString, NADescriptionCString);
    self->expandingPatternList = NASetCreate(NAHashCString, NADescriptionCString);

    return self;
}

NAMidiParserContext *NAMidiParserContextCreateCopy(NAMidiParserContext *self)
{
    NAMidiParserContext *copy = calloc(1, sizeof(NAMidiParserContext));

    memcpy(copy, self, sizeof(NAMidiParserContext));
    copy->contextIdList = NASetCreate(NAHashCString, NADescriptionCString);

    NAIterator *iterator = NASetGetIterator(self->contextIdList);
    while (iterator->hasNext(iterator)) {
        NASetAdd(copy->contextIdList, iterator->next(iterator));
    }

    copy->copy = true;
    return copy;
}

void NAMidiParserContextDestroy(NAMidiParserContext *self)
{
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
