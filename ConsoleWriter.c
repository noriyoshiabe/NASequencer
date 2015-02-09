#include "ConsoleWriter.h"

struct _ConsoleWriter {
    NAType _;
    CFMutableArrayRef tracksBuffer;
};

static void *__ConsoleWriterInit(void *self, ...)
{
    return self;
}

static void __ConsoleWriterDestroy(void *_self)
{
    ConsoleWriter *self = _self;
    if (self->tracksBuffer) {
        CFRelease(self->tracksBuffer);
    }
}

static void __ConsoleWriterVisitSequence(Sequence *elem){}
static void __ConsoleWriterVisitPattern(Pattern *elem){}
static void __ConsoleWriterVisitTimeTable(TimeTable *elem){}
static void __ConsoleWriterVisitTimeEvent(TimeEvent *elem){}
static void __ConsoleWriterVisitTempoEvent(TempoEvent *elem){}
static void __ConsoleWriterVisitMarkerEvent(MarkerEvent *elem){}
static void __ConsoleWriterVisitSoundSelectEvent(SoundSelectEvent *elem){}
static void __ConsoleWriterVisitNoteEvent(NoteEvent *elem){}

void ConsoleWriterWrite(ConsoleWriter *self, ParseContext *context)
{
    printf("\nParse result of %s\n\n", context->filepath);

    if (PARSE_ERROR_NOERROR != context->error.kind) {
        printf("[Error] --------------------------------------------------------------------------\n");
        printf("kind: %s\n", ParseError2String(context->error.kind));
        printf("filepath: %s\n", context->error.filepath);
        printf("message: %s\n", context->error.message);
        printf("location: L=%d C=%d - L=%d C=%d\n",
                context->error.location.firstLine, context->error.location.firstColumn,
                context->error.location.lastLine, context->error.location.lastColumn);
    }
    else {
    }

    printf("\n");
}

NADeclareVtbl(ConsoleWriter, NAType,
        __ConsoleWriterInit,
        __ConsoleWriterDestroy,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        );

NADeclareVtbl(ConsoleWriter, SequenceVisitor,
        __ConsoleWriterVisitSequence,
        __ConsoleWriterVisitPattern,
        __ConsoleWriterVisitTimeTable,
        __ConsoleWriterVisitTimeEvent,
        __ConsoleWriterVisitTempoEvent,
        __ConsoleWriterVisitMarkerEvent,
        __ConsoleWriterVisitSoundSelectEvent,
        __ConsoleWriterVisitNoteEvent,
        );

NADeclareClass(ConsoleWriter, NAType, SequenceVisitor);
