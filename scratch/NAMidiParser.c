#include "NAMidiParser.h"

#include "Parser.h"
#include "Lexer.h"
#include "ParserCallback.h"

#include <stdlib.h>

int yyparse(void *scanner, ParserCallback callback, ParserErrorCallback errorCallback);

struct _NAMidiParser {
    NAMidiParserError error;
};

static void NAMidiParserCallback(void *context, ParseLocation *location, Statement statement, ...);
static void NAMidiParserErrorCallback(void *context, ParseLocation *location, const char *message);

NAMidiParser *NAMidiParserCreate()
{
    return calloc(1, sizeof(NAMidiParser));
}

void NAMidiParserDestroy(NAMidiParser *self)
{
    free(self);
}

bool NAMidiParserExecuteParse(NAMidiParser *self, const char *filepath)
{
    void *scanner;
    bool ret = false;

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        self->error.kind = NAMidiParserErrorKindFileNotFound;
        return ret;
    }

    yylex_init_extra(self, &scanner);

    YY_BUFFER_STATE state = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);

    if (yyparse(scanner, NAMidiParserCallback, NAMidiParserErrorCallback)) {
        goto ERROR;
    }

    ret = true;

ERROR:
    yy_delete_buffer(state, scanner);
    yylex_destroy(scanner);
    fclose(fp);

    return ret;
}

void NAMidiParserRender(NAMidiParser *self, void *view, NAMidiParserRenderHandler handler)
{
}

static void NAMidiParserCallback(void *context, ParseLocation *location, Statement statement, ...)
{
    NAMidiParser *self = context;
    printf("statment=%d\n", statement);
}

static void NAMidiParserErrorCallback(void *context, ParseLocation *location, const char *message)
{
    NAMidiParser *self = context;
    printf("line=%d column=%d %s\n", location->line, location->column, message);
}
