#include "NAMidiParser.h"
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"

#include <stdlib.h>

extern int NAMidi_parse(yyscan_t scanner);

static bool NAMidiParserParseFile(Parser *self, const char *filepath, ParseResult *result);
static void NAMidiParserDestroy(Parser *self);

struct _NAMidiParser {
    Parser interface;
};

Parser *NAMidiParserCreate()
{
    NAMidiParser *self = calloc(1, sizeof(NAMidiParser));
    self->interface.parseFile = NAMidiParserParseFile;
    self->interface.destroy = NAMidiParserDestroy;
    return (Parser *)self;
}

static bool NAMidiParserParseFile(Parser *self, const char *filepath, ParseResult *result)
{
    NAMidi_parse(NULL);
    return true;
}

static void NAMidiParserDestroy(Parser *self)
{
    free(self);
}
