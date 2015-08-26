#include "Parser.h"
#include "NAMidiParser.h"
#include "NAMidiLexer.h"

#include <stdlib.h>

extern int NAMidi_parse(yyscan_t scanner);
extern void NAMidi_set_column(int column_no, yyscan_t scanner);

typedef struct _ParserInterface {
    int (*init_extra)(YY_EXTRA_TYPE extra, yyscan_t* scanner);
    int (*destroy)(yyscan_t scanner);
    YY_BUFFER_STATE (*scan_string)(const char *string, yyscan_t scanner);
    void (*set_lineno)(int lineno, yyscan_t scanner);
    void (*set_column)(int column_no, yyscan_t scanner);
    int (*parse)(yyscan_t scanner);
    void (*flush_buffer)(YY_BUFFER_STATE state, yyscan_t scanner);
} ParserInterface;

struct _Parser {
    yyscan_t scanner;
    const ParserInterface *parser;
    int lineno;
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
            NAMidi__scan_string,
            NAMidi_set_lineno,
            NAMidi_set_column,
            NAMidi_parse,
            NAMidi__flush_buffer
        }
    }
};

Parser *ParserCreate(Syntax syntax, void *handler)
{
    Parser *self = calloc(1, sizeof(Parser));
    for (int i = 0; i < sizeof(parserTable) / sizeof(parserTable[0]); ++i) {
        if (parserTable[i].syntax == syntax) {
            self->parser = &parserTable[i].interface;
            break;
        }
    }

    self->parser->init_extra(handler, &self->scanner);
    return self;
}

void ParserDestroy(Parser *self)
{
    self->parser->destroy(self->scanner);
    free(self);
}

void ParserScanString(Parser *self, char *line)
{
    YY_BUFFER_STATE state = self->parser->scan_string(line, self->scanner);
    self->parser->set_lineno(++self->lineno, self->scanner);
    self->parser->set_column(0, self->scanner);

    if (self->parser->parse(self->scanner)) {
        self->parser->flush_buffer(state, self->scanner);
    }
}
