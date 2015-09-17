#include "Exporter.h"
#include "NAMidi.h"
#include "SMFWriter.h"
#include "NAArray.h"
#include "NASet.h"

#include <stdlib.h>

struct _Exporter {
    Sequence *sequence;
};


Exporter *ExporterCreate(Sequence *sequence)
{
    Exporter *self = calloc(1, sizeof(Exporter));
    self->sequence = SequenceRetain(sequence);
    return self;
}

void ExporterDestroy(Exporter *self)
{
    SequenceRelease(self->sequence);
    free(self);
}

bool ExporterWriteToSMF(Exporter *self, const char *filepath)
{
    SMFWriter *writer = SMFWriterCreate();
    if (!SMFWriterOpenFile(writer, filepath)) {
        SMFWriterDestroy(writer);
        return false;
    }

    int count;
    MidiEvent **events;

    count = NAArrayCount(self->sequence->events);
    events = NAArrayGetValues(self->sequence->events);

    NASet *noteOffEvents = NASetCreate(NAHashAddress, NULL);
    NAArray *toWrite = NAArrayCreate(((count * 2 / 1024) + 1) * 1024, NULL);

    for (int i = 0; i < count; ++i) {
        NAArrayAppend(toWrite, events[i]);

        if (MidiEventTypeNote == events[i]->type) {
            NoteEvent *noteOn = (NoteEvent *)events[i];

            NoteEvent *noteOff = MidiEventAlloc(MidiEventTypeNote, noteOn->tick + noteOn->gatetime, sizeof(NoteEvent) - sizeof(MidiEvent));
            noteOff->channel = noteOn->channel;
            noteOff->noteNo = noteOn->noteNo;
            noteOff->velocity = 0;

            NASetAdd(noteOffEvents, noteOff);
            NAArrayAppend(toWrite, noteOff);
        }
    }

    NAArraySort(toWrite, MidiEventComparator);

    count = NAArrayCount(toWrite);
    events = NAArrayGetValues(toWrite);

    SMFWriterSetResolution(writer, TimeTableResolution(self->sequence->timeTable));

    for (int i = 0; i < count; ++i) {
        switch (events[i]->type) {
        case MidiEventTypeNote:
            {
                NoteEvent *note = (NoteEvent *)events[i];
                if (NASetContains(noteOffEvents, note)) {
                    SMFWriterAppendNoteOff(writer, note->tick, note->channel, note->noteNo, note->velocity);
                } else {
                    SMFWriterAppendNoteOn(writer, note->tick, note->channel, note->noteNo, note->velocity);
                }
            }
            break;
        case MidiEventTypeTime:
            {
                TimeEvent *time = (TimeEvent *)events[i];
                SMFWriterAppendTime(writer, time->tick, time->numerator, time->denominator);
            }
            break;
        case MidiEventTypeKey:
            {
                KeyEvent *key = (KeyEvent *)events[i];
                uint8_t sf, mi;
                KeySignGetMidiExpression(key->keySign, &sf, &mi);
                SMFWriterAppendKey(writer, key->tick, sf, mi);
            }
            break;
        case MidiEventTypeTempo:
            {
                TempoEvent *tempo = (TempoEvent *)events[i];
                SMFWriterAppendTempo(writer, tempo->tick, tempo->tempo);
            }
            break;
        case MidiEventTypeMarker:
            {
                MarkerEvent *marker = (MarkerEvent *)events[i];
                SMFWriterAppendMarker(writer, marker->tick, marker->text);
            }
            break;
        case MidiEventTypeVoice:
            {
                VoiceEvent *voice = (VoiceEvent *)events[i];
                SMFWriterAppendControlChange(writer, voice->tick, voice->channel, 0x00, voice->msb);
                SMFWriterAppendControlChange(writer, voice->tick, voice->channel, 0x20, voice->lsb);
                SMFWriterAppendProgramChange(writer, voice->tick, voice->channel, voice->programNo);
            }
            break;
        case MidiEventTypeVolume:
            {
                VolumeEvent *event = (VolumeEvent *)events[i];
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 7, event->value);
            }
            break;
        case MidiEventTypePan:
            {
                PanEvent *event = (PanEvent *)events[i];
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 10, event->value);
            }
            break;
        case MidiEventTypeChorus:
            {
                ChorusEvent *event = (ChorusEvent *)events[i];
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 93, event->value);
            }
            break;
        case MidiEventTypeReverb:
            {
                ReverbEvent *event = (ReverbEvent *)events[i];
                SMFWriterAppendControlChange(writer, event->tick, event->channel, 91, event->value);
            }
            break;
        }
    }

    bool ret = SMFWriterSerialize(writer);

    NASetTraverse(noteOffEvents, free);
    NASetDestroy(noteOffEvents);
    NAArrayDestroy(toWrite);

    SequenceRelease(self->sequence);
    SMFWriterDestroy(writer);

    return ret;
}
