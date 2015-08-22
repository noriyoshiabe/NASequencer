#include "NAMidiParser.h"

#include "Parser.h"
#include "Lexer.h"
#include "ParserCallback.h"

#include <stdlib.h>
#include <libgen.h>
#include <limits.h>

int yyparse(void *scanner, const char *filepath, ParserCallback callback, ParserErrorCallback errorCallback);

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

    char *_filepath = realpath(filepath, NULL);
    _filepath = _filepath ? _filepath : strdup(filepath);
    self->filepaths[self->fileCount] = _filepath;
    self->filepaths[++self->fileCount] = NULL;

    FILE *fp = fopen(_filepath, "r");
    if (!fp) {
        self->error.kind = NAMidiParserErrorKindFileNotFound;
        self->error.message = strdup("include file is not found.");
        return ret;
    }

    yylex_init_extra(self, &scanner);

    YY_BUFFER_STATE state = yy_create_buffer(fp, YY_BUF_SIZE, scanner);
    yy_switch_to_buffer(state, scanner);

    if (yyparse(scanner, _filepath, NAMidiParserCallback, NAMidiParserErrorCallback)) {
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

typedef bool (*NAMidiParserStatemntParser)(NAMidiParser *self, ParseLocation *location, StatementType type, va_list argList);
static NAMidiParserStatemntParser statementParserTable[StatementTypeCount] = {NULL};

static bool NAMidiParserCallback(void *context, ParseLocation *location, StatementType type, ...)
{
    NAMidiParser *self = context;
    bool success = false;

    va_list argList;
    va_start(argList, type);

    NAMidiParserStatemntParser parser = statementParserTable[type];
    if (parser) {
        printf("statment=%s\n", StatementType2String(type));
        success = parser(self, location, type, argList);
    }
    else {
        printf("parser for statment=%s id not implemented.\n", StatementType2String(type));
        success = true;
    }

    if (!success && !self->error.filepath) {
        self->error.filepath = location->filepath;;
        self->error.line = location->line;
        self->error.column = location->column;
    }

    va_end(argList);

    return success;
}

static void NAMidiParserErrorCallback(void *context, ParseLocation *location, const char *message)
{
    NAMidiParser *self = context;

    self->error.kind = NAMidiParserErrorKindSyntaxError;
    self->error.message = strdup(message);
    self->error.filepath = location->filepath;
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

static bool parseInclude(NAMidiParser *self, ParseLocation *location, StatementType type, va_list argList)
{
    char filename[256];
    char *_filename = va_arg(argList, char *);
    int len = strlen(_filename);
    strncpy(filename, _filename + 1, len - 2);
    filename[len - 2] = '\0';

    char *directory = dirname((char *)location->filepath);

    char buf[PATH_MAX + 1];
    snprintf(buf, PATH_MAX + 1, "%s/%s", directory, filename);

    free(directory);

    return NAMidiParserExecuteParse(self, buf);
}

static void __attribute__((constructor)) initializeTable()
{
    statementParserTable[StatementTypeInclude] = parseInclude;
}

