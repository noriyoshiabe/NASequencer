#include "SequenceBuilder.h"
#include "NAUtil.h"
#include "NAByteBuffer.h"
#include "NAMap.h"
#include "NAArray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <libgen.h>

struct _SequenceBuilder {
    NAByteBuffer *songBuffer;
    NAMap *patternBuffers;
    NAArray *patternIdentifiers;
    NAByteBuffer *currentBuffer;
};

SequenceBuilder *SequenceBuilderCreate()
{
    SequenceBuilder *self = calloc(1, sizeof(SequenceBuilder));
    self->songBuffer = NAByteBufferCreate(1024);
    self->patternBuffers = NAMapCreate(NAHashCString, NADescriptionCString, NULL);
    self->patternIdentifiers = NAArrayCreate(16, NADescriptionCString);
    self->currentBuffer = self->songBuffer;
    return self;
}

void SequenceBuilderDestroy(SequenceBuilder *self)
{
    NAByteBufferDestroy(self->songBuffer);

    NAMapTraverseValue(self->patternBuffers, (void *)NAByteBufferDestroy);
    NAMapDestroy(self->patternBuffers);

    NAArrayTraverse(self->patternIdentifiers, free);
    NAArrayDestroy(self->patternIdentifiers);

    free(self);
}

bool SequenceBuilderBuild(SequenceBuilder *self, ParseResult *result)
{
    int type;
    printf("-- song buffer --\n");
    while (NAByteBufferReadInteger(self->songBuffer, &type)) {
        printf("statement: %s\n", StatementType2String(type));
    }

    NAArrayDescription(self->patternIdentifiers, stdout);
    int count = NAArrayCount(self->patternIdentifiers);
    char **identifiers = NAArrayGetValues(self->patternIdentifiers);
    for (int i = 0; i < count; ++i) {
        printf("-- pattern buffer [%s] --\n", identifiers[i]);

        NAByteBuffer *buffer = NAMapGet(self->patternBuffers, identifiers[i]);
        while (NAByteBufferReadInteger(buffer, &type)) {
            printf("statement: %s\n", StatementType2String(type));
        }
    }

    result->sequence = SequenceCreate();
    return true;
}

static bool SequenceBuilderIncludeFile(SequenceBuilder *self, ParseContext *context, const char *filename)
{
    char *directory = dirname((char *)context->location.filepath);
    char *fullPath = NAUtilBuildPathWithDirectory(directory, filename);

    bool success = ParserParseFileWithContext(fullPath, context);

    free(fullPath);
    return success;
}

static bool StatementHandlerProcess(void *receiver, ParseContext *context, StatementType type, ...)
{
    SequenceBuilder *self = receiver;

    va_list argList;
    va_start(argList, type);

    bool success = true;

    switch (type) {
    case StatementTypeResolution:
        NAByteBufferWriteInteger(self->currentBuffer, type);
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        break;
    case StatementTypeTitle:
        NAByteBufferWriteInteger(self->currentBuffer, type);
        NAByteBufferWriteString(self->currentBuffer, va_arg(argList, char *));
        break;
    case StatementTypeTempo:
        NAByteBufferWriteInteger(self->currentBuffer, type);
        NAByteBufferWriteFloat(self->currentBuffer, va_arg(argList, double));
        break;
    case StatementTypeTimeSign:
        NAByteBufferWriteInteger(self->currentBuffer, type);
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        break;
    case StatementTypeMeasure:
        NAByteBufferWriteInteger(self->currentBuffer, type);
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        break;
    case StatementTypeMarker:
    case StatementTypePattern:
        NAByteBufferWriteInteger(self->currentBuffer, type);
        NAByteBufferWriteString(self->currentBuffer, va_arg(argList, char *));
        break;
    case StatementTypePatternDefine:
        {
            char *patternIdentifier = strdup(va_arg(argList, char *));

            if (NAMapContainsKey(self->patternBuffers, patternIdentifier)) {
                printf("duplicate pattern identifier: %s\n", patternIdentifier);
                free(patternIdentifier);
                success = false;
                break;
            }

            NAByteBuffer *buffer = NAByteBufferCreate(1024);
            NAMapPut(self->patternBuffers, patternIdentifier, buffer);
            NAArrayAppend(self->patternIdentifiers, patternIdentifier);
            self->currentBuffer = buffer;
        }
        break;
    case StatementTypeEnd:
        // TODO Stack management
        if (self->currentBuffer == self->songBuffer) {
            printf("unexpected pattern end\n");
            success = false;
            break;
        }

        self->currentBuffer = self->songBuffer;
        break;
    case StatementTypeTrack:
    case StatementTypeChannel:
        NAByteBufferWriteInteger(self->currentBuffer, type);
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        break;
    case StatementTypeVoice:
        NAByteBufferWriteInteger(self->currentBuffer, type);
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        break;
    case StatementTypeVolume:
    case StatementTypePan:
    case StatementTypeChorus:
    case StatementTypeReverb:
    case StatementTypeTranspose:
    case StatementTypeKey:
        NAByteBufferWriteInteger(self->currentBuffer, type);
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        break;
    case StatementTypeNote:
        NAByteBufferWriteInteger(self->currentBuffer, type);
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        break;
    case StatementTypeRest:
        break;
    case StatementTypeInclude:
        success = SequenceBuilderIncludeFile(receiver, context, va_arg(argList, char *));
        break;
    default:
        break;
    }

    va_end(argList);

    return success;
}

static void StatementHandlerError(void *receiver, ParseContext *context, ParseError *error)
{
    printf("error: %s %s %d %d\n", ParseErrorKind2String(error->kind), error->location.filepath, error->location.line, error->location.column);
}

StatementHandler SequenceBuilderStatementHandler = {
    StatementHandlerProcess,
    StatementHandlerError
};
