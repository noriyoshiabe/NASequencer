#include "NAMidiParser.h"
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"

#include "NASet.h"
#include "NAStack.h"

#include <stdlib.h>

extern int NAMidi_parse(yyscan_t scanner);

static bool NAMidiParserParseFile(void *self, const char *filepath);
static void NAMidiParserDestroy(void *self);

struct _NAMidiParser {
    Parser interface;
    ParseResult *result;
    NASet *fileSet;
    NAStack *fileStack;
    char *currentFile;
};

Parser *NAMidiParserCreate(ParseResult *result)
{
    NAMidiParser *self = calloc(1, sizeof(NAMidiParser));
    self->interface.parseFile = NAMidiParserParseFile;
    self->interface.destroy = NAMidiParserDestroy;
    self->result = result;
    self->fileSet = NASetCreate(NAHashCString, NADescriptionCString);
    self->fileStack = NAStackCreate(4);
    return (Parser *)self;
}

static bool NAMidiParserParseFile(void *_self, const char *filepath)
{
    NAMidiParser *self = _self;

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        self->result->error.kind = ParseErrorKindFileNotFound;
        return false;
    }

    char *copiedFilePath;
    if (!(copiedFilePath = NASetGet(self->fileSet, (char *)filepath))) {
        copiedFilePath = strdup(filepath);
        NASetAdd(self->fileSet, copiedFilePath);
        NAArrayAppend(self->result->filepaths, copiedFilePath);
    }

    if (self->currentFile) {
        NAStackPush(self->fileStack, self->currentFile);
    }

    self->currentFile = copiedFilePath;

    yyscan_t scanner;
    NAMidi_lex_init_extra(self, &scanner);
    YY_BUFFER_STATE state = NAMidi__create_buffer(fp, YY_BUF_SIZE, scanner);
    NAMidi__switch_to_buffer(state, scanner);

    bool success = 0 == NAMidi_parse(scanner);

    self->currentFile = NAStackPop(self->fileStack);
    
    NAMidi__delete_buffer(state, scanner);
    NAMidi_lex_destroy(scanner);
    fclose(fp);
    
    return true;
}

static void NAMidiParserDestroy(void *_self)
{
    NAMidiParser *self = _self;

    NAStackDestroy(self->fileStack);
    NASetDestroy(self->fileSet);
    free(self);
}

bool NAMidiParserProcess(void *self, int line, int column, StatementType type, va_list argList)
{
    return true;
}

void NAMidiParserError(void *self, int line, int column, ParseErrorKind errorKind)
{
}
