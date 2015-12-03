#include "NAMidiParser.h"

#include <stdlib.h>

typedef struct _NAMidiParser {
    DSLParser parser;
    ParseContext *context;
} NAMidiParser;

static Node *NAMidiParserParse(void *self, const char *filepath)
{
    // TODO
}

static void NAMidiParserDestroy(void *self)
{
    free(self);
}

DSLParser *NAMidiParserCreate(ParseContext *context)
{
    NAMidiParser *self = calloc(1, sizeof(NAMidiParser));
    self->context = context;
    self->parser.parse = NAMidiParserParse;
    self->parser.destroy = NAMidiParserParse;
    return self;
}

int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, void **node, const char *message)
{
    NAMidiParser *self = NAMidi_get_extra(scanner);
    self->context->appendError(self->context, &((FileLocation){(char *)filepath, yylloc->first_line, yylloc->first_column}, GeneralParseErrorSyntaxError, NAMidi_get_text(scanner), NULL);
    return 0;
}
