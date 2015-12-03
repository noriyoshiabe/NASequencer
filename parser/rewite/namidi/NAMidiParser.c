#include "NAMidiParser.h"
#include "NAUtil.h"
#include "NAMidi_yacc.h"
#include "NAMidi_lex.h"

#include <stdlib.h>

typedef struct _NAMidiParser {
    DSLParser parser;
    ParseContext *context;
} NAMidiParser;

static Node *NAMidiParserParseInternal(NAMidiParser *self, const char *filepath)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        return NULL;
    }

    self->context->appendFile(self->context, fullpath);

    yyscan_t scanner;
    NAMidi_lex_init_extra(self, &scanner);
    YY_BUFFER_STATE state = NAMidi__create_buffer(fp, YY_BUF_SIZE, scanner);
    NAMidi__switch_to_buffer(state, scanner);

    Node *node = NULL;
    NAMidi_parse(scanner, fullpath, (void **)&node);

    NAMidi__delete_buffer(state, scanner);
    NAMidi_lex_destroy(scanner);
    fclose(fp);

    return node;
}

static Node *NAMidiParserParse(void *self, const char *filepath)
{
    char *fullpath = NAUtilGetRealPath(filepath);
    Node *node = NAMidiParserParseInternal(self, fullpath);
    if (!node) {
        self->context->appendError(self->context, NULL, GeneralParseErrorFileNotFound, filepath, NULL);
    } 
    free(fullpath);
    return node;
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
    self->parser.destroy = NAMidiParserDestroy;
    return self;
}

int NAMidi_error(YYLTYPE *yylloc, yyscan_t scanner, const char *filepath, void **node, const char *message)
{
    NAMidiParser *self = NAMidi_get_extra(scanner);
    self->context->appendError(self->context, &((FileLocation){(char *)filepath, yylloc->first_line, yylloc->first_column}, GeneralParseErrorSyntaxError, NAMidi_get_text(scanner), NULL);
    return 0;
}
