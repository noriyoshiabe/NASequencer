#include "NAMidiParser.h"
#include "NAMidiPreprocessor.h"
#include "NAMidiAST.h"
#include "NAIO.h"
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"
#include "NASet.h"
#include "NAMap.h"

#include <stdlib.h>
#include <libgen.h>

typedef struct _NAMidiParser {
    DSLParser parser;
    ParseContext *context;
    char *currentFilepath;
} NAMidiParser;

extern int NAMidi_parse(yyscan_t scanner, void **node);

static Node *NAMidiParserParse(void *_self, const char *filepath)
{
    NAMidiParser *self = _self; 

    NAMidiPreprocessor *preprocessor = NAMidiPreprocessorCreate(self->context);
    FILE *fp = NAMidiPreprocessorScanFile(preprocessor, filepath);
    NAMidiPreprocessorDestroy(preprocessor);

    yyscan_t scanner;
    NAMidi_lex_init_extra(self, &scanner);
    YY_BUFFER_STATE state = NAMidi__create_buffer(fp, YY_BUF_SIZE, scanner);
    NAMidi__switch_to_buffer(state, scanner);

    Node *node = NULL;
    NAMidi_parse(scanner, (void **)&node);

    NAMidi__delete_buffer(state, scanner);
    NAMidi_lex_destroy(scanner);

    fclose(fp);

    return node;
}

static void NAMidiParserDestroy(void *_self)
{
    NAMidiParser *self = _self;
    free(self);
}

DSLParser *NAMidiParserCreate(ParseContext *context)
{
    NAMidiParser *self = calloc(1, sizeof(NAMidiParser));
    self->context = context;
    self->parser.parse = NAMidiParserParse;
    self->parser.destroy = NAMidiParserDestroy;
    return (DSLParser *)self;
}

void NAMidiParserSetCurrentFilepath(void *_self, const char *filepath)
{
    NAMidiParser *self = _self;
    self->currentFilepath = self->context->appendFile(self->context, filepath);
}

char *NAMidiParserGetCurrentFilepath(void *_self)
{
    NAMidiParser *self = _self;
    return self->currentFilepath;
}

void NAMidiParserSyntaxError(void *_self, FileLocation *location, const char *token)
{
    NAMidiParser *self = _self; 
    self->context->appendError(self->context, location, GeneralParseErrorSyntaxError, token, NULL);
}

void NAMidiParserUnExpectedEOF(void *_self, FileLocation *location)
{
    NAMidiParser *self = _self; 
    self->context->appendError(self->context, location, NAMidiParseErrorUnexpectedEOF, NULL);
}
