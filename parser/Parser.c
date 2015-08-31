#include "Parser.h"
#include "NAMidiParser.h"
#include "NAMidiLexer.h"
#include "SequenceBuilder.h"
#include "NAUtil.h"

#include <stdlib.h>

extern int NAMidi_parse(yyscan_t scanner);

typedef enum {
    SyntaxNAMidi,
} Syntax;

typedef struct _ParserInterface {
    int (*lex_init_extra)(YY_EXTRA_TYPE extra, yyscan_t* scanner);
    int (*lex_destroy)(yyscan_t scanner);
    YY_BUFFER_STATE (*create_buffer)(FILE *file, int size, yyscan_t yyscanner);
    void (*switch_to_buffer)(YY_BUFFER_STATE new_buffer, yyscan_t yyscanner);
    void (*delete_buffer)(YY_BUFFER_STATE b, yyscan_t yyscanner);
    int (*parse)(yyscan_t scanner);
} ParserInterface;

struct _Parser {
    yyscan_t scanner;
    const ParserInterface *parser;
    ParseContext context;
};

static const struct {
    Syntax syntax;
    ParserInterface interface;
} parserTable[] = {
    {
        SyntaxNAMidi,
        {
            NAMidi_lex_init_extra,
            NAMidi_lex_destroy,
            NAMidi__create_buffer,
            NAMidi__switch_to_buffer,
            NAMidi__delete_buffer,
            NAMidi_parse,
        }
    }
};

static ParseContext *ParseContextCeate(void *receiver, StatementHandler *handler, ParseResult *result)
{
    ParseContext *context = calloc(1, sizeof(ParseContext));
    context->receiver = receiver;
    context->handler = handler;
    context->fileSet = NASetCreate(NAHashCString, NADescriptionCString);
    context->result = result;
    return context;
}

static void ParseContextDestroy(ParseContext *context)
{
    NASetDestroy(context->fileSet);
    free(context);
}

bool ParserParseFile(const char *filepath, ParseResult *result)
{
    result->filepaths = NAArrayCreate(4, NADescriptionCString);

    SequenceBuilder *builder = SequenceBuilderCreate();
    ParseContext *context = ParseContextCeate(builder, &SequenceBuilderStatementHandler, result);
    char *fullpath = NAUtilGetRealPath(filepath);

    bool success = ParserParseFileWithContext(fullpath, context) && SequenceBuilderBuild(builder, result);

    SequenceBuilderDestroy(builder);
    ParseContextDestroy(context);
    free(fullpath);

    return success;
}

bool ParserParseFileWithContext(const char *filepath, ParseContext *context)
{
    // TODO filetype check
    const ParserInterface *parser = &parserTable[0].interface;
    if (!parser) {
        context->result->error.kind = ParseErrorKindUnsupportedFileType;
        return false;
    }

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        context->result->error.kind = ParseErrorKindFileNotFound;
        return false;
    }

    char *copiedFilePath;
    if (!(copiedFilePath = NASetGet(context->fileSet, (char *)filepath))) {
        copiedFilePath = strdup(filepath);
        NASetAdd(context->fileSet, copiedFilePath);
        NAArrayAppend(context->result->filepaths, copiedFilePath);
    }

    const char *previousFilepath = context->location.filepath;
    context->location.filepath = copiedFilePath;

    yyscan_t scanner;
    parser->lex_init_extra(context, &scanner);
    YY_BUFFER_STATE state = parser->create_buffer(fp, YY_BUF_SIZE, scanner);
    parser->switch_to_buffer(state, scanner);

    bool success = 0 == parser->parse(scanner);

    context->location.filepath = previousFilepath;

    parser->delete_buffer(state, scanner);
    parser->lex_destroy(scanner);

    fclose(fp);

    return success;
}
