#include "ConsoleWriter.h"
#include <NACFHelper.h>

#define TRACK_BUFFER_NUM 18
#define TIME_TABLE_INDEX 0
#define META_INFO_INDEX 1
#define CHANNEL2INDEX(channel) (channel + 1)
#define INDEX2CHANNEL(index) (index - 1)

struct _ConsoleWriter {
    NAType _;
    CFMutableArrayRef tracksBuffer;
    TimeTable *timeTable;
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

static void resetTrackBuffer(ConsoleWriter *self)
{
    if (self->tracksBuffer) {
        CFRelease(self->tracksBuffer);
    }

    self->tracksBuffer = CFArrayCreateMutable(NULL, TRACK_BUFFER_NUM, &kCFTypeArrayCallBacks);
    for (int i = 0; i < TRACK_BUFFER_NUM; ++i) {
        CFStringRef string = CFStringCreateMutable(NULL, 0);
        CFArrayAppendValue(self->tracksBuffer, string);
        CFRelease(string);
    }
}

static void writeTrackBuffer(ConsoleWriter *self)
{
    CFIndex count = CFArrayGetCount(self->tracksBuffer);
    for (int i = 0; i < count; ++i) {
        CFStringRef string = CFArrayGetValueAtIndex(self->tracksBuffer, i);
        if (0 < CFStringGetLength(string)) {
            CFShow(string);
        }
    }
}

static void writeTracks(ConsoleWriter *self, TimeTable *timeTable, CFMutableArrayRef events)
{
    CFIndex count;

    resetTrackBuffer(self);

    CFMutableStringRef string = (CFMutableStringRef)CFArrayGetValueAtIndex(self->tracksBuffer, TIME_TABLE_INDEX);

    CFStringAppendFormat(string, NULL, CFSTR("\n"));
    CFStringAppendFormat(string, NULL, CFSTR("[Time table]\n"));
    CFStringAppendFormat(string, NULL, CFSTR("----------------------------------------------------------------------------------\n"));

    SequenceElementAccept(timeTable, self);

    count = CFArrayGetCount(events);
    for (int i = 0; i < count; ++i) {
        void *event = (void *)CFArrayGetValueAtIndex(events, i);
        SequenceElementAccept(event, self);
    }

    count = CFArrayGetCount(self->tracksBuffer);
    for (int i = 1; i < count; ++i) {
        CFMutableStringRef string = (CFMutableStringRef)CFArrayGetValueAtIndex(self->tracksBuffer, i);
        if (0 < CFStringGetLength(string)) {
            CFMutableStringRef header = CFStringCreateMutable(NULL, 0);
            CFStringAppendFormat(header, NULL, CFSTR("\n"));
            if (META_INFO_INDEX == i) {
                CFStringAppendFormat(header, NULL, CFSTR("[Meta info]\n"));
            }
            else {
                CFStringAppendFormat(header, NULL, CFSTR("[Channel %d]\n"), INDEX2CHANNEL(i));
            }
            CFStringAppendFormat(header, NULL, CFSTR("----------------------------------------------------------------------------------\n"));

            CFStringInsert(string, 0, header);
            CFRelease(header);
        }
    }

    writeTrackBuffer(self);
    printf("\n");
}

static void __ConsoleWriterVisitSequence(void *_self, Sequence *elem)
{
    ConsoleWriter *self = _self;

    printf("\n");
    printf("[Sequence] title: %s  resolution: %d\n", NACFString2CString(elem->title), elem->resolution);
    printf("==================================================================================\n");

    writeTracks(self, elem->timeTable, elem->events);
}

static void __ConsoleWriterVisitPattern(void *_self, Pattern *elem)
{
    ConsoleWriter *self = _self;

    CFIndex size = CFStringGetLength(elem->name) + 1;
    char *cstr = malloc(size);
    CFStringGetCString(elem->name, cstr, size, kCFStringEncodingUTF8);
    printf("\n");
    printf("[Pattern] name: %s  length: %d\n", cstr, elem->length);
    printf("==================================================================================\n");
    free(cstr);

    writeTracks(self, elem->timeTable, elem->events);
}

static void __ConsoleWriterVisitTimeTable(void *_self, TimeTable *elem)
{
    ConsoleWriter *self = _self;
    self->timeTable = elem;

    CFIndex count;

    count = CFArrayGetCount(elem->timeEvents);
    for (int i = 0; i < count; ++i) {
        void *event = (void *)CFArrayGetValueAtIndex(elem->timeEvents, i);
        SequenceElementAccept(event, self);
    }

    count = CFArrayGetCount(elem->tempoEvents);
    for (int i = 0; i < count; ++i) {
        void *event = (void *)CFArrayGetValueAtIndex(elem->tempoEvents, i);
        SequenceElementAccept(event, self);
    }
}

static void __ConsoleWriterVisitTimeEvent(void *_self, TimeEvent *elem)
{
    ConsoleWriter *self = _self;
    CFMutableStringRef string = (CFMutableStringRef)CFArrayGetValueAtIndex(self->tracksBuffer, TIME_TABLE_INDEX);
    Location l = TimeTableTick2Location(self->timeTable, elem->_.tick);
    CFStringAppendFormat(string, NULL, CFSTR("%03d:%02d:%03d: [Time signature] %d/%d\n"), l.m, l.b, l.t, elem->numerator, elem->denominator);
}

static void __ConsoleWriterVisitTempoEvent(void *_self, TempoEvent *elem)
{
    ConsoleWriter *self = _self;
    CFMutableStringRef string = (CFMutableStringRef)CFArrayGetValueAtIndex(self->tracksBuffer, TIME_TABLE_INDEX);
    Location l = TimeTableTick2Location(self->timeTable, elem->_.tick);
    CFStringAppendFormat(string, NULL, CFSTR("%03d:%02d:%03d: [Tempo] %.2f\n"), l.m, l.b, l.t, elem->tempo);
}

static void __ConsoleWriterVisitMarkerEvent(void *_self, MarkerEvent *elem)
{
    ConsoleWriter *self = _self;
    CFMutableStringRef string = (CFMutableStringRef)CFArrayGetValueAtIndex(self->tracksBuffer, META_INFO_INDEX);
    Location l = TimeTableTick2Location(self->timeTable, elem->_.tick);
    CFStringAppendFormat(string, NULL, CFSTR("%03d:%02d:%03d: [Marker] %@\n"), l.m, l.b, l.t, elem->text);
}

static void __ConsoleWriterVisitSoundSelectEvent(void *_self, SoundSelectEvent *elem)
{
    ConsoleWriter *self = _self;
    CFMutableStringRef string = (CFMutableStringRef)CFArrayGetValueAtIndex(self->tracksBuffer, CHANNEL2INDEX(elem->channel));
    Location l = TimeTableTick2Location(self->timeTable, elem->_.tick);
    CFStringAppendFormat(string, NULL, CFSTR("%03d:%02d:%03d: [Sound select] channel:%d msb:%d lsb:%d program no:%d\n"),
            l.m, l.b, l.t, elem->channel, elem->msb, elem->lsb, elem->programNo);
}

static void __ConsoleWriterVisitNoteEvent(void *_self, NoteEvent *elem)
{
    ConsoleWriter *self = _self;
    CFMutableStringRef string = (CFMutableStringRef)CFArrayGetValueAtIndex(self->tracksBuffer, elem->channel + 1);
    Location l = TimeTableTick2Location(self->timeTable, elem->_.tick);
    CFStringAppendFormat(string, NULL, CFSTR("%03d:%02d:%03d: [Note] channel:%d note no:%d velocity:%d gatetime:%d\n"),
            l.m, l.b, l.t, elem->channel, elem->noteNo, elem->velocity, elem->gatetime);
}

void __ConsoleWriterRender(void *self, ParseContext *context)
{
    printf("\nParse result of %s\n\n", NACFString2CString(context->filepath));

    if (context->error) {
        printf("[Error] --------------------------------------------------------------------------\n");
        printf("kind: %s\n", ParseError2String(context->error->kind));
        printf("filepath: %s\n", NACFString2CString(context->error->filepath));
        printf("message: %s\n", NACFString2CString(context->error->message));
        printf("location: L=%d C=%d - L=%d C=%d\n",
                context->error->location.firstLine, context->error->location.firstColumn,
                context->error->location.lastLine, context->error->location.lastColumn);
    }
    else {
        SequenceElementAccept(context->sequence, self);

        CFIndex count = CFDictionaryGetCount(context->patterns);
        CFTypeRef *keysTypeRef = (CFTypeRef *)malloc(count * sizeof(CFTypeRef));
        CFDictionaryGetKeysAndValues(context->patterns, (const void **)keysTypeRef, NULL);
        for (int i = 0; i < count; ++i) {
            Pattern *pattern = (Pattern *)CFDictionaryGetValue(context->patterns, keysTypeRef[i]);
            SequenceElementAccept(pattern, self);
        }

        free(keysTypeRef);
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

NADeclareVtbl(ConsoleWriter, ParseContextView, __ConsoleWriterRender);

NADeclareClass(ConsoleWriter, NAType, SequenceVisitor, ParseContextView);