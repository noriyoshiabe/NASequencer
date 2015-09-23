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
    int from;
    int length;
    int columnStep;

    Sequence *sequence;
    Track tracks[16];
};

static NAMidiObserverCallbacks PianoRollViewNAMidiObserverCallbacks;


PianoRollView *PianoRollViewCreate(NAMidi *namidi)
{
    PianoRollView *self = calloc(1, sizeof(PianoRollView));
    self->namidi = namidi;
    self->from = -1;
    self->length = -1;
    self->columnStep = 120;
    NAMidiAddObserver(self->namidi, self, &PianoRollViewNAMidiObserverCallbacks);

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

    NAMidiRemoveObserver(self->namidi, self);
    free(self);
}

void PianoRollViewSetFrom(PianoRollView *self, int from)
{
    self->from = from;
}

void PianoRollViewSetLength(PianoRollView *self, int length)
{
    self->length = length;
}

#define MEASURE_COLUMN_OFFSET 14

static void PianoRollViewRenderMeasure(PianoRollView *self, RenderContext *context);
static void PianoRollViewRenderTrack(PianoRollView *self, RenderContext *context, Track *track);

void PianoRollViewRender(PianoRollView *self)
{
    RenderContext context;
    for (int i = 0; i < 16; ++i) {
        context.indices[i] = 0;
    }
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &context.w);

    int measure = MAX(1, self->from);

    int length = -1 == self->length
        ? TimeTableLength(self->sequence->timeTable)
        : MIN(self->length, TimeTableLength(self->sequence->timeTable));

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
        for (int i = 0; i < 16; ++i) {
            PianoRollViewRenderTrack(self, &context, &self->tracks[i]);
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

    for (int tick = context->from; tick < context->to; tick += self->columnStep) {
        Location location = TimeTableTick2Location(self->sequence->timeTable, tick);

        if (1 == location.b && 0 == location.t) {
            char number[4];
            snprintf(number, 4, "%d", location.m);
            strncpy(buffer + MEASURE_COLUMN_OFFSET + offset, number, strlen(number));
        }

        ++offset;
    }

    printf("%s\n", buffer);
    for (int i = 0; i < MEASURE_COLUMN_OFFSET + offset; ++i) {
        printf("=");
    }
    printf("\n");
}

static void PianoRollViewRenderTrack(PianoRollView *self, RenderContext *context, Track *track)
{
    if (0 == NAArrayCount(track->events)) {
        return;
    }

    int lineCount = track->noteRange.high - track->noteRange.low + 1;
    char buffer[lineCount][context->w.ws_col + 1];
    for (int i = 0; i < lineCount; ++i) {
        memset(buffer[i], ' ', context->w.ws_col);
        buffer[i][context->w.ws_col] = '\0';
        buffer[i][MEASURE_COLUMN_OFFSET - 2] = '|';
        buffer[i][4] = '|';

        if (0 == i) {
            char channel[4];
            snprintf(channel, 4, "CH%d", track->channel);
            strncpy(buffer[i], channel, strlen(channel));
        }
    }

    //for (int i = 0; i < lineCount; ++i) {
    //    char noteLabel[]
    //    buffer[i][5]
    //}

    int offset = 0;

    for (int tick = context->from; tick < context->to; tick += self->columnStep) {
        Location location = TimeTableTick2Location(self->sequence->timeTable, tick);
        if (1 == location.b && 0 == location.t) {
            for (int i = 0; i < lineCount; ++i) {
                buffer[i][MEASURE_COLUMN_OFFSET + offset] = '.';
            }
        }

        ++offset;
    }

    for (int i = 0; i < lineCount; ++i) {
        printf("%s\n", buffer[i]);
    }
}

static void PianoRollViewNAMidiOnParseFinish(void *receiver, Sequence *sequence)
{
    PianoRollView *self = receiver;
    
    if (self->sequence) {
        SequenceRelease(self->sequence);
    }

    self->sequence = SequenceRetain(sequence);

    int count = NAArrayCount(sequence->events);
    MidiEvent **events = NAArrayGetValues(sequence->events);
    for (int i = 0; i < count; ++i) {
        MidiEvent *event = events[i];
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
        if (0 != track->noteRange.high % 12) {
            track->noteRange.high = (track->noteRange.high / 12) * 12 + 12;
        }

        if (track->noteRange.low == track->noteRange.high) {
            track->noteRange.high += 12;
        }

        track->noteRange.high = MIN(127, track->noteRange.high);
    }

    PianoRollViewRender(self);
}

static void PianoRollViewNAMidiOnParseError(void *receiver, ParseError *error)
{
}

static NAMidiObserverCallbacks PianoRollViewNAMidiObserverCallbacks = {
    PianoRollViewNAMidiOnParseFinish,
    PianoRollViewNAMidiOnParseError
};
