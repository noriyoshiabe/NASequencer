#include "SequenceBuilder.h"
#include "NAUtil.h"
#include "NAByteBuffer.h"
#include "NAMap.h"
#include "NAArray.h"
#include "NAStack.h"

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

typedef struct _BuildContext {
    NAByteBuffer *buffer;
    NAMap *patternContexts;
    NAArray *patternIdentifiers;
} BuildContext;

struct _SequenceBuilder {
    BuildContext *context;
    NAStack *contextStack;
};

static BuildContext *BuildContextCreate()
{
    BuildContext *self = calloc(1, sizeof(BuildContext));
    self->buffer = NAByteBufferCreate(1024);
    self->patternContexts = NAMapCreate(NAHashCString, NADescriptionCString, NULL);
    self->patternIdentifiers = NAArrayCreate(16, NADescriptionCString);
    return self;
}

static void BuildContextDestroy(BuildContext *self)
{
    NAByteBufferDestroy(self->buffer);

    NAMapTraverseValue(self->patternContexts, (void *)BuildContextDestroy);
    NAMapDestroy(self->patternContexts);

    NAArrayTraverse(self->patternIdentifiers, free);
    NAArrayDestroy(self->patternIdentifiers);

    free(self);
}

SequenceBuilder *SequenceBuilderCreate()
{
    SequenceBuilder *self = calloc(1, sizeof(SequenceBuilder));
    self->context = BuildContextCreate();
    self->contextStack = NAStackCreate(4);
    return self;
}

void SequenceBuilderDestroy(SequenceBuilder *self)
{
    BuildContextDestroy(self->context);
    NAStackDestroy(self->contextStack);

    free(self);
}

static void BuildContextDump(BuildContext *self, char *name, int indent)
{
    if (name) {
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("-- pattern [%s] buffer --\n", name);
    }
    else {
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("-- song buffer --\n");
    }

    StatementHeader *header;
    void *data;

    while (NAByteBufferReadData(self->buffer, &header, sizeof(StatementHeader))) {
        for (int i = 0; i < indent; ++i) printf(" ");
        printf("statement: %s - %d [%s %d %d]\n",
                StatementType2String(header->type),
                header->length, header->location.filepath, header->location.line, header->location.column);
        NAByteBufferReadData(self->buffer, &data, header->length);
    }

    int count = NAArrayCount(self->patternIdentifiers);
    char **identifiers = NAArrayGetValues(self->patternIdentifiers);
    for (int i = 0; i < count; ++i) {
        BuildContext *context = NAMapGet(self->patternContexts, identifiers[i]);
        BuildContextDump(context, identifiers[i], indent + 2);
    }
}

bool SequenceBuilderBuild(SequenceBuilder *self, ParseResult *result)
{
    BuildContextDump(self->context, NULL, 0);
    result->sequence = SequenceCreate();
    return true;
}

static bool StatementHandlerProcess(void *receiver, ParseContext *parseContext, StatementType type, ...)
{
    SequenceBuilder *self = receiver;

    StatementHeader header;
    header.type = type;
    header.location = parseContext->location;

    va_list argList;
    va_start(argList, type);

    bool success = true;

    switch (type) {
    case StatementTypeResolution:
        header.length = sizeof(int);
        NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
        NAByteBufferWriteInteger(self->context->buffer, va_arg(argList, int));
        break;
    case StatementTypeTitle:
        {
            char *string = va_arg(argList, char *);
            header.length = strlen(string) + 1;
            NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
            NAByteBufferWriteString(self->context->buffer, string);
        }
        break;
    case StatementTypeTempo:
        header.length = sizeof(float);
        NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
        NAByteBufferWriteFloat(self->context->buffer, va_arg(argList, double));
        break;
    case StatementTypeTimeSign:
        header.length = sizeof(int) * 2;
        NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
        NAByteBufferWriteInteger(self->context->buffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->context->buffer, va_arg(argList, int));
        break;
    case StatementTypeMeasure:
        header.length = sizeof(int);
        NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
        NAByteBufferWriteInteger(self->context->buffer, va_arg(argList, int));
        break;
    case StatementTypeMarker:
    case StatementTypePattern:
        {
            char *string = va_arg(argList, char *);
            header.length = strlen(string) + 1;
            NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
            NAByteBufferWriteString(self->context->buffer, string);
        }
        break;
    case StatementTypePatternDefine:
        {
            char *patternIdentifier = strdup(va_arg(argList, char *));

            if (NAMapContainsKey(self->context->patternContexts, patternIdentifier)) {
                parseContext->result->error.kind = ParseErrorKindDuplicatePatternIdentifier;
                parseContext->result->error.location = parseContext->location;
                free(patternIdentifier);
                success = false;
                break;
            }

            BuildContext *buildContext = BuildContextCreate();
            NAMapPut(self->context->patternContexts, patternIdentifier, buildContext);
            NAArrayAppend(self->context->patternIdentifiers, patternIdentifier);

            NAStackPush(self->contextStack, self->context);
            self->context = buildContext;
        }
        break;
    case StatementTypeEnd:
        {
            BuildContext *buildContext = NAStackPop(self->contextStack);
            if (!buildContext) {
                parseContext->result->error.kind = ParseErrorKindUnexpectedEnd;
                parseContext->result->error.location = parseContext->location;
                success = false;
                break;
            }

            self->context = buildContext;
        }
        break;
    case StatementTypeTrack:
    case StatementTypeChannel:
        header.length = sizeof(int);
        NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
        NAByteBufferWriteInteger(self->context->buffer, va_arg(argList, int));
        break;
    case StatementTypeVoice:
        header.length = sizeof(int) * 3;
        NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
        NAByteBufferWriteInteger(self->context->buffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->context->buffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->context->buffer, va_arg(argList, int));
        break;
    case StatementTypeVolume:
    case StatementTypePan:
    case StatementTypeChorus:
    case StatementTypeReverb:
    case StatementTypeTranspose:
    case StatementTypeKey:
        header.length = sizeof(int);
        NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
        NAByteBufferWriteInteger(self->context->buffer, va_arg(argList, int));
        break;
    case StatementTypeNote:
        header.length = sizeof(int) * 6;
        NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
        NAByteBufferWriteInteger(self->context->buffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->context->buffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->context->buffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->context->buffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->context->buffer, va_arg(argList, int));
        NAByteBufferWriteInteger(self->context->buffer, va_arg(argList, int));
        break;
    case StatementTypeRest:
        header.length = sizeof(int);
        NAByteBufferWriteData(self->context->buffer, &header, sizeof(StatementHeader));
        NAByteBufferWriteInteger(self->context->buffer, va_arg(argList, int));
        break;
    case StatementTypeInclude:
        {
            char *filename = va_arg(argList, char *);
            char *directory = dirname((char *)parseContext->location.filepath);
            char *fullPath = NAUtilBuildPathWithDirectory(directory, filename);

            if (NASetContains(parseContext->fileSet, fullPath)) {
                parseContext->result->error.kind = ParseErrorKindCircularFileInclude;
                parseContext->result->error.location = parseContext->location;
                free(fullPath);
                success = false;
                break;
            }

            success = ParserParseFileWithContext(fullPath, parseContext);
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
