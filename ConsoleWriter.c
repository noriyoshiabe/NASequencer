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

void ConsoleWriterWrite(ConsoleWriter *self, ParseContext *parseContext)
{
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
