#include "NAMidiDriver.h"

#include <stdlib.h>

typedef struct _NAMidiDriver {
    Driver driver;
    ParseContext *context;
} NAMidiDriver;

Driver *NAMidiDriverCreate(ParseContext *context)
{
    NAMidiDriver *self = calloc(1, sizeof(NAMidiDriver));
    self->context = context;
    return (Driver *)self;
}
