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

typedef struct _StatementHeader {
    StatementType type;
    ParseLocation location;
    int length;
} StatementHeader;

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
    StatementHeader *header;
    void *data;
    printf("-- song buffer --\n");
    while (NAByteBufferReadData(self->songBuffer, &header, sizeof(StatementHeader))) {
        printf("statement: %s - %d [%s %d %d]\n",
                StatementType2String(header->type),
                header->length, header->location.filepath, header->location.line, header->location.column);
        NAByteBufferReadData(self->songBuffer, &data, header->length);
    }

    int count = NAArrayCount(self->patternIdentifiers);
    char **identifiers = NAArrayGetValues(self->patternIdentifiers);
    for (int i = 0; i < count; ++i) {
        printf("-- pattern buffer [%s] --\n", identifiers[i]);

        NAByteBuffer *buffer = NAMapGet(self->patternBuffers, identifiers[i]);
        while (NAByteBufferReadData(buffer, &header, sizeof(StatementHeader))) {
            printf("statement: %s - %d [%s %d %d]\n",
                    StatementType2String(header->type),
                    header->length, header->location.filepath, header->location.line, header->location.column);
            NAByteBufferReadData(buffer, &data, header->length);
        }
    }

    result->sequence = SequenceCreate();
    return true;
}

static bool StatementHandlerProcess(void *receiver, ParseContext *context, StatementType type, ...)
{
    SequenceBuilder *self = receiver;

    StatementHeader header;
    header.type = type;
    header.location = context->location;

    va_list argList;
    va_start(argList, type);

    bool success = true;

    switch (type) {
    case StatementTypeResolution:
        header.length = sizeof(int);
        NAByteBufferWriteData(self->currentBuffer, &header, sizeof(StatementHeader));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        break;
    case StatementTypeTitle:
        {
            char *string = va_arg(argList, char *);
            header.length = strlen(string) + 1;
            NAByteBufferWriteData(self->currentBuffer, &header, sizeof(StatementHeader));
            NAByteBufferWriteString(self->currentBuffer, string);
        }
        break;
    case StatementTypeTempo:
        header.length = sizeof(float);
        NAByteBufferWriteData(self->currentBuffer, &header, sizeof(StatementHeader));
        NAByteBufferWriteFloat(self->currentBuffer, va_arg(argList, double));
        break;
    case StatementTypeTimeSign:
        header.length = sizeof(int) * 2;
        NAByteBufferWriteData(self->currentBuffer, &header, sizeof(StatementHeader));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        break;
    case StatementTypeMeasure:
        header.length = sizeof(int);
        NAByteBufferWriteData(self->currentBuffer, &header, sizeof(StatementHeader));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        break;
    case StatementTypeMarker:
    case StatementTypePattern:
        {
            char *string = va_arg(argList, char *);
            header.length = strlen(string) + 1;
            NAByteBufferWriteData(self->currentBuffer, &header, sizeof(StatementHeader));
            NAByteBufferWriteString(self->currentBuffer, string);
        }
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
        header.length = sizeof(int);
        NAByteBufferWriteData(self->currentBuffer, &header, sizeof(StatementHeader));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        break;
    case StatementTypeVoice:
        header.length = sizeof(int) * 3;
        NAByteBufferWriteData(self->currentBuffer, &header, sizeof(StatementHeader));
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
        header.length = sizeof(int);
        NAByteBufferWriteData(self->currentBuffer, &header, sizeof(StatementHeader));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        break;
    case StatementTypeNote:
        header.length = sizeof(int) * 6;
        NAByteBufferWriteData(self->currentBuffer, &header, sizeof(StatementHeader));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        break;
    case StatementTypeRest:
        header.length = sizeof(int);
        NAByteBufferWriteData(self->currentBuffer, &header, sizeof(StatementHeader));
        NAByteBufferWriteInteger(self->currentBuffer, va_arg(argList, int));
        break;
    case StatementTypeInclude:
        {
            char *filename = va_arg(argList, char *);
            char *directory = dirname((char *)context->location.filepath);
            char *fullPath = NAUtilBuildPathWithDirectory(directory, filename);

            if (NASetContains(context->fileSet, fullPath)) {
                printf("circular refence of file '%s'\n", filename);
                free(fullPath);
                success = false;
                break;
            }

            success = ParserParseFileWithContext(fullPath, context);
            free(fullPath);
        }
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
