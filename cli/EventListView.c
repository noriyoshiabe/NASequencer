#include "EventListView.h"
#include "NoteTable.h"

#include <stdlib.h>
#include <sys/param.h>

struct _EventListView {
    NAMidi *namidi;
    int channel;
    int from;
    int length;
    Sequence *sequence;
};

EventListView *EventListViewCreate(NAMidi *namidi)
{
    EventListView *self = calloc(1, sizeof(EventListView));
    self->namidi = namidi;
    self->channel = -1;
    self->from = -1;
    self->length = -1;
    return self;
}

void EventListViewDestroy(EventListView *self)
{
    if (self->sequence) {
        SequenceRelease(self->sequence);
    }

    free(self);
}

void EventListViewSetChannel(EventListView *self, int channel)
{
    self->channel = channel;
}

void EventListViewSetFrom(EventListView *self, int from)
{
    self->from = from;
}

void EventListViewSetLength(EventListView *self, int length)
{
    self->length = length;
}

static const char *NoteNo2NoteLabel(int noteNo)
{
    const char *labels[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
    };

    return labels[noteNo % 12];
}

static int NoteNo2Octave(int noteNo)
{
    return noteNo / 12 - 2;
}

void EventListViewRender(EventListView *self)
{
    if (!self->sequence) {
        printf("sequence not built.\n");
        return;
    }

    const struct {
        const char *label;
        int width;
    } table[] = {
        {"Position", 10},
        {"Type", 6},
        {"Ch", 2},
        {"Note no", 8},
        {"Gatetime", 8},
        {"Velocity", 8},
        {"Other", 40},
    };

    const int tableSize = sizeof(table) / sizeof(table[0]);
    char texts[tableSize][64];

    for (int i = 0; i < tableSize; ++i) {
        printf("%-*s%s", table[i].width, table[i].label, i == tableSize - 1 ? "\n" : " | ");
    }

    for (int i = 0; i < tableSize; ++i) {
        for (int j = 0; j < table[i].width; ++j) {
            printf("-");
        }

        printf("%s", i == tableSize - 1 ? "\n" : "-|-");
    }

    int measure = MAX(1, self->from);
    int from = TimeTableTickByMeasure(self->sequence->timeTable, measure);
    int to = -1 == self->length
        ? TimeTableLength(self->sequence->timeTable)
        : TimeTableTickByMeasure(self->sequence->timeTable, measure + self->length);

    int count = NAArrayCount(self->sequence->events);
    MidiEvent **events = NAArrayGetValues(self->sequence->events);

    for (int i = 0; i < count; ++i) {
        MidiEvent *event = events[i];

        if (event->tick < from || to <= event->tick) {
            continue;
        }

        if (-1 != self->channel) {
            switch (event->type) {
            case MidiEventTypeTempo:
            case MidiEventTypeTime:
            case MidiEventTypeKey:
            case MidiEventTypeMarker:
                continue;
            default:
                if (self->channel != ((ChannelEvent *)event)->channel) {
                    continue;
                }
                break;
            }
        }

        Location l = TimeTableTick2Location(self->sequence->timeTable, event->tick);
        sprintf(texts[0], "%03d:%02d:%03d", l.m, l.b, l.t);
        sprintf(texts[1], "%s", MidiEventType2String(event->type));
        texts[2][0] = '\0';
        texts[3][0] = '\0';
        texts[4][0] = '\0';
        texts[5][0] = '\0';
        texts[6][0] = '\0';

        switch (event->type) {
        case MidiEventTypeNote:
            {
                NoteEvent *_event = (void *)event;
                sprintf(texts[2], "%d", _event->channel);
                sprintf(texts[3], "%03d:%s%d", _event->noteNo,
                        NoteNo2NoteLabel(_event->noteNo), NoteNo2Octave(_event->noteNo));
                sprintf(texts[4], "%d", _event->gatetime);
                sprintf(texts[5], "%d", _event->velocity);
            }
            break;
        case MidiEventTypeTempo:
            {
                TempoEvent *_event = (void *)event;
                sprintf(texts[6], "%.2f", _event->tempo);
            }
            break;
        case MidiEventTypeTime:
            {
                TimeEvent *_event = (void *)event;
                sprintf(texts[6], "%d/%d", _event->numerator, _event->denominator);
            }
            break;
        case MidiEventTypeKey:
            {
                KeyEvent *_event = (void *)event;
                sprintf(texts[6], "%s", KeySign2String(NoteTableGetKeySignByMidiExpression(_event->sf, _event->mi)));
            }
            break;
        case MidiEventTypeMarker:
            {
                MarkerEvent *_event = (void *)event;
                sprintf(texts[6], "%s", _event->text);
            }
            break;
        case MidiEventTypeVoice:
            {
                VoiceEvent *_event = (void *)event;
                sprintf(texts[2], "%d", _event->channel);
                sprintf(texts[6], "MSB: %d  LSB: %d  Prg: %d", _event->msb, _event->lsb, _event->programNo);
            }
            break;
        case MidiEventTypeVolume:
            {
                VolumeEvent *_event = (void *)event;
                sprintf(texts[2], "%d", _event->channel);
                sprintf(texts[6], "Volume: %d", _event->value);
            }
            break;
        case MidiEventTypePan:
            {
                PanEvent *_event = (void *)event;
                sprintf(texts[2], "%d", _event->channel);
                sprintf(texts[6], "Pan: %d", _event->value);
            }
            break;
        case MidiEventTypeChorus:
            {
                ChorusEvent *_event = (void *)event;
                sprintf(texts[2], "%d", _event->channel);
                sprintf(texts[6], "Chorus: %d", _event->value);
            }
            break;
        case MidiEventTypeReverb:
            {
                ReverbEvent *_event = (void *)event;
                sprintf(texts[2], "%d", _event->channel);
                sprintf(texts[6], "Reverb: %d", _event->value);
            }
            break;
        case MidiEventTypeSynth:
            {
                SynthEvent *_event = (void *)event;
                sprintf(texts[2], "%d", _event->channel);
                sprintf(texts[6], "Synth: %s", _event->identifier);
            }
            break;
        }

        for (int j = 0; j < tableSize; ++j) {
            char text[128];
            int width = table[j].width;
            int limit = width + 1;

            switch (j) {
            case 1:
            case 3:
            case 6:
                snprintf(text, limit, "%-*s", width, texts[j]);
                break;
            default:
                snprintf(text, limit, "%*s", width, texts[j]);
                break;
            }

            printf("%s%s", text, j == sizeof(table)/sizeof(table[0]) - 1 ? "\n" : " | ");
        }
    }
}

void EventListViewSetSequence(EventListView *self, Sequence *sequence)
{
    if (self->sequence) {
        SequenceRelease(self->sequence);
    }

    self->sequence = SequenceRetain(sequence);
}
