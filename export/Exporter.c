#include "Exporter.h"
#include "NAMidi.h"
#include "SMFWriter.h"
#include "NAArray.h"

#include <stdlib.h>

struct _Exporter {
    const char *filepath;
    const char *soundSource;
    NAMidi *namidi;
    Sequence *sequence;
};

static void ExporterNAMidiOnParseFinish(void *receiver, Sequence *sequence)
{
    Exporter *self = receiver;
    self->sequence = sequence;
}

static void ExporterNAMidiOnParseError(void *receiver, ParseError *error)
{
    printf("%s %s - %d:%d\n", ParseErrorKind2String(error->kind), error->location.filepath, error->location.line, error->location.column);
}

static NAMidiObserverCallbacks ExporterNAMidiObserverCallbacks = {
    ExporterNAMidiOnParseFinish,
    ExporterNAMidiOnParseError
};


Exporter *ExporterCreate(const char *filepath, const char *soundSource)
{
    Exporter *self = calloc(1, sizeof(Exporter));
    self->filepath = filepath;
    self->soundSource = soundSource;
    self->namidi = NAMidiCreate();
    NAMidiAddObserver(self->namidi, self, &ExporterNAMidiObserverCallbacks);
    return self;
}

void ExporterDestroy(Exporter *self)
{
    NAMidiDestroy(self->namidi);
    free(self);
}

bool ExporterWriteToSMF(Exporter *self, const char *filepath)
{
    NAMidiParse(self->namidi, self->filepath);

    if (!self->sequence) {
        return false;
    }

    SMFWriter *writer = SMFWriterCreate(filepath);

    int count;
    MidiEvent **events;

    count = NAArrayCount(self->sequence->events);
    events = NAArrayGetValues(self->sequence->events);

    NAArray *noteOffEvents = NAArrayCreate(count, NULL);
    NAArray *toWrite = NAArrayCreate(((count * 2 / 1024) + 1) * 1024, NULL);

    for (int i = 0; i < count; ++i) {
        NAArrayAppend(toWrite, events[i]);

        if (MidiEventTypeNote == events[i]->type) {
            NoteEvent *noteOn = (NoteEvent *)events[i];
            NoteEvent *noteOff = MidiEventAlloc(MidiEventTypeNote, noteOn->tick + noteOn->gatetime, sizeof(NoteEvent) - sizeof(MidiEvent));
            NAArrayAppend(noteOffEvents, noteOff);
            NAArrayAppend(toWrite, noteOff);
        }
    }

    NAArraySort(toWrite, MidiEventComparator);

    count = NAArrayCount(toWrite);
    events = NAArrayGetValues(toWrite);

    SMFWriterSetResolution(writer, TimeTableResolution(self->sequence->timeTable));




    bool ret = SMFWriterSerialize(writer);

    NAArrayTraverse(noteOffEvents, free);
    NAArrayDestroy(noteOffEvents);
    NAArrayDestroy(toWrite);

    SequenceRelease(self->sequence);
    SMFWriterDestroy(writer);

    return ret;
}
