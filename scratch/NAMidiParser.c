#include "NAMidiParser.h"

#include "Parser.h"
#include "Lexer.h"
#include "ParserCallback.h"

#include <stdlib.h>

int yyparse(void *scanner, ParserCallback callback, ParserErrorCallback errorCallback);

typedef struct _Statement {
    ParseLocation location;
    StatementType type;
    union {
        int i;
        float f;
        char *s;
    } values[4];
} Statement;

typedef struct _StatementList {
    Statement *array;
    int count;
    int capacity;
} StatementList;

static StatementList *StatementListCreate();
static Statement *StatementListAlloc(StatementList *self);
static void StatementListDestroy(StatementList *self);

typedef enum {
    NAMidiParserStateSong,
    NAMidiParserStatePattern,
    NAMidiParserStatePhrase,
} NAMidiParserState;

struct _NAMidiParser {
    NAMidiParserError error;
    StatementList *songStatements;
    char **filepaths;
    int fileCount;

    struct {
        NAMidiParserState state;
        const StatementList *currentStatements;
        const char *currentFilepath;
    } parseContext;
};

static bool NAMidiParserCallback(void *context, ParseLocation *location, StatementType type, ...);
static void NAMidiParserErrorCallback(void *context, ParseLocation *location, const char *message);

NAMidiParser *NAMidiParserCreate()
{
    NAMidiParser *self = calloc(1, sizeof(NAMidiParser));
    self->songStatements = StatementListCreate();

    self->parseContext.state = NAMidiParserStateSong;
    self->parseContext.currentStatements = self->songStatements;

    return self;
}

void NAMidiParserDestroy(NAMidiParser *self)
{
    if (self->error.message) {
        free(self->error.message);
    }

    for (int i = 0; i < self->fileCount; ++i) {
        free(self->filepaths[i]);
    }

    if (self->filepaths) {
        free(self->filepaths);
    }

    StatementListDestroy(self->songStatements);
    free(self);
}

bool NAMidiParserExecuteParse(NAMidiParser *self, const char *filepath)
{
    void *scanner;
    bool ret = false;

    self->filepaths = realloc(self->filepaths, (self->fileCount + 2) * sizeof(char *));
    self->filepaths[self->fileCount] = strdup(filepath);
    self->parseContext.currentFilepath = self->filepaths[self->fileCount];
    self->filepaths[++self->fileCount] = NULL;

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        self->error.kind = NAMidiParserErrorKindFileNotFound;
        self->error.filepath = self->parseContext.currentFilepath;
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

const NAMidiParserError *NAMidiParserGetError(NAMidiParser *self)
{
    return (const NAMidiParserError *)&self->error;
}

const char **NAMidiParserGetFilepaths(NAMidiParser *self)
{
    return (const char **)self->filepaths;
}

static bool NAMidiParserCallback(void *context, ParseLocation *location, StatementType type, ...)
{
    NAMidiParser *self = context;

    va_list argList;
    va_start(argList, type);

    // TODO dispatch
    if (false) {
        self->error.line = location->line;
        self->error.column = location->column;
        return false;
    }

    va_end(argList);

    return true;
}

static void NAMidiParserErrorCallback(void *context, ParseLocation *location, const char *message)
{
    NAMidiParser *self = context;

    self->error.kind = NAMidiParserErrorKindSyntaxError;
    self->error.message = strdup(message);
    self->error.filepath = self->parseContext.currentFilepath;
    self->error.line = location->line;
    self->error.column = location->column;
}


static StatementList *StatementListCreate()
{
    StatementList *self = calloc(1, sizeof(StatementList));
    self->capacity = 32;
    self->array = calloc(self->capacity, sizeof(Statement));
    return self;
}

static Statement *StatementListAlloc(StatementList *self)
{
    if (self->capacity <= self->count) {
        self->capacity *= 2;
        self->array = realloc(self->array, self->capacity);
    }

    return &self->array[self->count++];
}

static void StatementListDestroy(StatementList *self)
{
    for (int i = 0; i < self->count; ++i) {
        switch (self->array[i].type) {
        case StatementTypeTitle:
        case StatementTypeMarker:
        case StatementTypePattern:
        case StatementTypePatternDefine:
        case StatementTypePhrase:
        case StatementTypePhraseDefine:
        case StatementTypeKey:
        case StatementTypeNote:
            free(self->array[i].values[0].s);
            break;
        default:
            break;
        }
    }

    free(self->array);
    free(self);
}
