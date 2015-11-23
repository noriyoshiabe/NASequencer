#include "ABCParser.h"
#include "ABC_yacc.h"
#include "ABC_lex.h"
#include "ABCExpression.h"

#include "NAUtil.h"
#include "NASet.h"

#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

extern int ABC_parse(yyscan_t scanner, const char *filepath, Expression **expression);

struct _ABCParser {
    DSLParser interface;
    ParserCallbacks *callbacks;
    void *receiver;
    SequenceBuilder *builder;

    NASet *fileSet;
};

static bool ABCParserParseFileInternal(ABCParser *self, const char *filepath, Expression **expression);

static bool ABCParserParseFile(void *_self, const char *filepath)
{
    ABCParser *self = _self;

    Expression *expression;

    if (!ABCParserParseFileInternal(self, filepath, &expression)) {
        ABCParserError(self, NULL, GeneralParseErrorFileNotFound, filepath, NULL);
        return false;
    }

    bool success = ExpressionParse(expression, self, NULL);
    ExpressionDestroy(expression);
    return success;
}

static bool ABCParserParseFileInternal(ABCParser *self, const char *filepath, Expression **expression)
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

    yyscan_t scanner;
    ABC_lex_init_extra(self, &scanner);
    YY_BUFFER_STATE state = ABC__create_buffer(fp, YY_BUF_SIZE, scanner);
    ABC__switch_to_buffer(state, scanner);

    bool success = 0 == ABC_parse(scanner, copiedFilePath, expression);

    ABC__delete_buffer(state, scanner);
    ABC_lex_destroy(scanner);
    fclose(fp);

    return success;
}

int ABC_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, Expression **expression, const char *message)
{
    ParseLocation location = {(char *)filepath, yylloc->first_line, yylloc->first_column};
    ABCParserError(ABC_get_extra(scanner), &location, GeneralParseErrorSyntaxError, ABC_get_text(scanner), NULL);
    return 0;
}

void ABCParserError(ABCParser *self, ParseLocation *location, int code, ...)
{
    va_list argList;
    va_start(argList, code);
    ParseError *error = ParseErrorCreateWithArgs(location, code, argList);
    va_end(argList);

    self->callbacks->onParseError(self->receiver, error);
}

SequenceBuilder *ABCParserGetBuilder(ABCParser *self)
{
    return self->builder;
}

static void ABCParserDestroy(void *_self)
{
    ABCParser *self = _self;
    NASetTraverse(self->fileSet, free);
    NASetDestroy(self->fileSet);
    free(self);
}

DSLParser *ABCParserCreate(SequenceBuilder *builder, ParserCallbacks *callbacks, void *receiver)
{
    ABCParser *self = calloc(1, sizeof(ABCParser));
    self->interface.parseFile = ABCParserParseFile;
    self->interface.destroy = ABCParserDestroy;
    self->callbacks = callbacks;
    self->receiver = receiver;
    self->builder = builder;
    self->fileSet = NASetCreate(NAHashCString, NADescriptionCString);
    return (DSLParser *)self;
}

ABCParserContext *ABCParserContextCreate()
{
    ABCParserContext *self = calloc(1, sizeof(ABCParserContext));
    self->noteTable = NoteTableCreate(BaseNote_C, false, false, ModeMajor);
    self->channel = 1;
    return self;
}

void ABCParserContextDestroy(ABCParserContext *self)
{
    NoteTableRelease(self->noteTable);
    free(self);
}

int ABCParserContextGetLength(ABCParserContext *self)
{
    int length = 0;
    for (int i = 0; i < 16; ++i) {
        length = MAX(length, self->channels[i].tick);
    }
    return length;
}
