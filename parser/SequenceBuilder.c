#include "SequenceBuilder.h"

#include <stdio.h>
#include <stdlib.h>

struct _SequenceBuilder {
};

SequenceBuilder *SequenceBuilderCreate()
{
    SequenceBuilder *self = calloc(1, sizeof(SequenceBuilder));
    return self;
}

void SequenceBuilderDestroy(SequenceBuilder *self)
{
    free(self);
}

bool SequenceBuilderBuild(SequenceBuilder *self, ParseResult *result)
{
    result->sequence = SequenceCreate();
    return true;
}

static bool StatementHandlerProcess(void *receiver, ParseContext *context, StatementType type, va_list argList)
{
    printf("statement: %s\n", StatementType2String(type));
    return true;
}

static void StatementHandlerError(void *receiver, ParseContext *context, ParseError *error)
{
    printf("error: %s %s %d %d\n", ParseErrorKind2String(error->kind), error->location.filepath, error->location.line, error->location.column);
}

StatementHandler SequenceBuilderStatementHandler = {
    StatementHandlerProcess,
    StatementHandlerError
};
