#include "NAMidiParser.h"

#include <stdlib.h>

typedef struct _NAMidiParser {
    Parser interface;
} NAMidiParser;

Parser *NAMidiParserCreate(SequenceBuilder *builder, ParserCallbacks *callbacks, void *receiver)
{
    NAMidiParser *self = calloc(1, sizeof(NAMidiParser));
    return (Parser *)self;
}
