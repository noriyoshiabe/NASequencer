#include "PianoRollView.h"
#include "NAArray.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <alloca.h>
#include <sys/ioctl.h>
#include <sys/param.h>

typedef struct _Track {
    int channel;
    NAArray *events;
    struct {
        int low;
        int high;
    } noteRange;
} Track;

typedef struct _RenderContext {
    struct winsize w;
    int from;
    int to;
    int indices[16];
} RenderContext;

struct _PianoRollView {
    NAMidi *namidi;
    int channel;
    int from;
    int length;
    int columnStep;

    Sequence *sequence;
    Track tracks[16];
};

PianoRollView *PianoRollViewCreate(NAMidi *namidi)
{
    PianoRollView *self = calloc(1, sizeof(PianoRollView));
    self->namidi = namidi;
    self->channel = -1;
    self->from = -1;
    self->length = -1;
    self->columnStep = 120;

    for (int i = 0; i < 16; ++i) {
        self->tracks[i].channel = i + 1;
        self->tracks[i].events = NAArrayCreate(32, NULL);
        self->tracks[i].noteRange.low = 127;
        self->tracks[i].noteRange.high = 0;
    }

    return self;
}

void PianoRollViewDestroy(PianoRollView *self)
{
    for (int i = 0; i < 16; ++i) {
        NAArrayDestroy(self->tracks[i].events);
    }

    if (self->sequence) {
        SequenceRelease(self->sequence);
    }

    free(self);
}

void PianoRollViewSetChannel(PianoRollView *self, int channel)
{
    self->channel = channel;
}

void PianoRollViewSetFrom(PianoRollView *self, int from)
{
    self->from = from;
}

void PianoRollViewSetLength(PianoRollView *self, int length)
{
    self->length = length;
}

void PianoRollViewSetStep(PianoRollView *self, int step)
{
    self->columnStep = step;
}

#define MEASURE_COLUMN_OFFSET 14

static void PianoRollViewRenderMeasure(PianoRollView *self, RenderContext *context);
static void PianoRollViewRenderTrack(PianoRollView *self, RenderContext *context, Track *track);

void PianoRollViewRender(PianoRollView *self)
{
    if (!self->sequence) {
        printf("sequence not built.\n");
        return;
    }

    printf("\n");

    RenderContext context;
    for (int i = 0; i < 16; ++i) {
        context.indices[i] = 0;
    }
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &context.w);

    int measure = MAX(1, self->from);

    int tick = TimeTableTickByMeasure(self->sequence->timeTable, measure);
    int to = -1 == self->length
        ? TimeTableLength(self->sequence->timeTable)
        : TimeTableTickByMeasure(self->sequence->timeTable, measure + self->length);

    while (tick < to) {
        int tickTo = tick + self->columnStep * (context.w.ws_col - MEASURE_COLUMN_OFFSET);
        Location location = TimeTableTick2Location(self->sequence->timeTable, tickTo);
        tickTo = TimeTableTickByMeasure(self->sequence->timeTable, location.m);

        context.from = tick;
        context.to = MIN(to, tickTo);

        PianoRollViewRenderMeasure(self, &context);

        bool first = true;
        for (int i = 0; i < 16; ++i) {
            if (-1 != self->channel && i + 1 != self->channel) {
                continue;
            }

            if (0 < NAArrayCount(self->tracks[i].events)) {
                if (!first) {
                    int column = MEASURE_COLUMN_OFFSET + (context.to - context.from) / self->columnStep;
                    for (int j = 0; j < column; ++j) {
                        printf("-");
                    }
                    printf("\n");
                }

                PianoRollViewRenderTrack(self, &context, &self->tracks[i]);
                first = false;
            }
        }

        measure = location.m;
        tick = TimeTableTickByMeasure(self->sequence->timeTable, measure);

        printf("\n");
    }
}

static void PianoRollViewRenderMeasure(PianoRollView *self, RenderContext *context)
{
    char *buffer = alloca(context->w.ws_col + 1);
    memset(buffer, ' ', context->w.ws_col);
    buffer[context->w.ws_col] = '\0';
    buffer[MEASURE_COLUMN_OFFSET - 2] = '|';

    int offset = 0;
    int prevMeasure = 0;

    for (int tick = context->from; tick < context->to; tick += self->columnStep) {
        Location location = TimeTableTick2Location(self->sequence->timeTable, tick);

        if (prevMeasure != location.m) {
            char number[4];
            snprintf(number, 4, "%d", location.m);
            strncpy(buffer + MEASURE_COLUMN_OFFSET + offset, number, strlen(number));
        }

        ++offset;
        prevMeasure = location.m;
    }

    printf("%s\n", buffer);
    for (int i = 0; i < MEASURE_COLUMN_OFFSET + offset; ++i) {
        printf("=");
    }
    printf("\n");
}

static const char *NoteNo2NoteLabel(int noteNo)
{
    const char *labels[] = {
        "C-2", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
        "C-1", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
        "C0" , "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
        "C1",  "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
        "C2",  "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
        "C3",  "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
        "C4",  "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
        "C5",  "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
        "C6",  "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
        "C7",  "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
        "C8",  "C#", "D", "D#", "E", "F", "F#",
    };

    return labels[noteNo];
}

static void PianoRollViewRenderTrack(PianoRollView *self, RenderContext *context, Track *track)
{
    int lineCount = track->noteRange.high - track->noteRange.low + 1;
    char buffer[lineCount][context->w.ws_col + 1];
    for (int i = 0; i < lineCount; ++i) {
        memset(buffer[i], ' ', context->w.ws_col);
        buffer[i][context->w.ws_col] = '\0';
        buffer[i][MEASURE_COLUMN_OFFSET - 2] = '|';
        buffer[i][4] = '|';

        if (0 == i) {
            char channel[5];
            snprintf(channel, 5, "Ch%d", track->channel);
            strncpy(buffer[i], channel, strlen(channel));
        }
    }

    for (int i = 0; i < lineCount; ++i) {
        int noteNo = track->noteRange.high - i;
        char label[8];
        sprintf(label, "%03d:%s", noteNo, NoteNo2NoteLabel(noteNo));
        strncpy(&buffer[i][5], label, strlen(label));
    }

    int offset = 0;
    int prevMeasure = 0;

    for (int tick = context->from; tick < context->to; tick += self->columnStep) {
        Location location = TimeTableTick2Location(self->sequence->timeTable, tick);
        if (prevMeasure != location.m) {
            for (int i = 0; i < lineCount; ++i) {
                buffer[i][MEASURE_COLUMN_OFFSET + offset] = '.';
            }
        }

        ++offset;
        prevMeasure = location.m;
    }

    int *index = &context->indices[track->channel - 1];
    int count = NAArrayCount(track->events);
    NoteEvent **events = NAArrayGetValues(track->events);
    for (; *index < count; ++(*index)) {
        NoteEvent *note = events[*index];
        if (context->to <= note->tick) {
            break;
        }

        for (int tick = note->tick; tick < note->tick + note->gatetime; tick += self->columnStep) {
            if (tick < context->from) {
                continue;
            }

            int offset = MEASURE_COLUMN_OFFSET + (tick - context->from) / self->columnStep;
            int line = track->noteRange.high - note->noteNo;
            if (tick == note->tick) {
                buffer[line][offset] = 'x';
            }
            else {
                buffer[line][offset] = '-';
            }
        }
    }

    for (int i = 0; i < lineCount; ++i) {
        printf("%s\n", buffer[i]);
    }
}

void PianoRollViewSetSequence(PianoRollView *self, Sequence *sequence)
{
    if (self->sequence) {
        SequenceRelease(self->sequence);
        for (int i = 0; i < 16; ++i) {
            NAArrayDestroy(self->tracks[i].events);
            self->tracks[i].events = NAArrayCreate(32, NULL);
            self->tracks[i].noteRange.low = 127;
            self->tracks[i].noteRange.high = 0;
        }
    }

    self->sequence = SequenceRetain(sequence);

    NAIterator *iterator = NAArrayGetIterator(sequence->events);
    while (iterator->hasNext(iterator)) {
        MidiEvent *event = iterator->next(iterator);
        if (MidiEventTypeNote == event->type) {
            NoteEvent *note = (NoteEvent *)event;
            Track *track = &self->tracks[note->channel - 1];
            track->noteRange.low = MIN(track->noteRange.low, note->noteNo);
            track->noteRange.high = MAX(track->noteRange.high, note->noteNo);
            NAArrayAppend(track->events, event);
        }
    }

    for (int i = 0; i < 16; ++i) {
        Track *track = &self->tracks[i];
        track->noteRange.low = (track->noteRange.low / 12) * 12;

        if (12 > track->noteRange.high - track->noteRange.low) {
            track->noteRange.high = track->noteRange.low + 12;
        }

        track->noteRange.high = MIN(127, track->noteRange.high);
    }
}
