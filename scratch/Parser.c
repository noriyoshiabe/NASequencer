#include "Parser.h"
#include "NAMidiParser.h"
#include "NAMidiLexer.h"

#include <stdlib.h>

extern int NAMidi_parse(yyscan_t scanner);

struct _Parser {
    yyscan_t scanner;
};

Parser *ParserCreate(Syntax syntax, void *handler)
{
    Parser *self = calloc(1, sizeof(Parser));
    NAMidi_lex_init_extra(handler, &self->scanner);
    return self;
}

void ParserDestroy(Parser *self)
{
    NAMidi_lex_destroy(self->scanner);
    free(self);
}

void ParserScanBuffer(Parser *self, char *buffer, int length)
{
    YY_BUFFER_STATE state = NAMidi__scan_buffer(buffer, length, self->scanner);
    if (NAMidi_parse(self->scanner)) {
        NAMidi__flush_buffer(state, self->scanner);
    }
}
