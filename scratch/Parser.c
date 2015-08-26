#include "Parser.h"
#include "NAMidiParser.h"
#include "NAMidiLexer.h"

#include <stdlib.h>

extern int NAMidi_parse(yyscan_t scanner);
extern void NAMidi_set_column(int column_no, yyscan_t scanner);

struct _Parser {
    yyscan_t scanner;
    int lineno;
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

void ParserScanString(Parser *self, char *line)
{
    YY_BUFFER_STATE state = NAMidi__scan_string(line, self->scanner);
    NAMidi_set_column(0, self->scanner);
    NAMidi_set_lineno(++self->lineno, self->scanner);

    if (NAMidi_parse(self->scanner)) {
        NAMidi__flush_buffer(state, self->scanner);
    }
}
