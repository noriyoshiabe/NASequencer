#include "Parser.h"
#include "ParseErrorCode.h"
#include "NAArray.h"
#include "MidiEvent.h"
#include "NALog.h"
#include "NAAlloc.h"
#include "NAIO.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

typedef struct _SequenceBuilderImpl {
    SequenceBuilder interface;
    NAArray *events;
    int id;
    int resolution;
    int length;
} SequenceBuilderImpl;

static SequenceBuilder *SequenceBuilderCreate();
static int __MidiEventComparator(const void *_event1, const void *_event2);
static void __MidiEventDump(MidiEvent *self);
static int __ParseErrorComparator(const void *_error1, const void *_error2);

static struct option _options[] = {
    { "silent", no_argument, NULL, 's'},
    { "error-count", required_argument, NULL, 'e'},

    {NULL, 0, NULL, 0}
};

int main(int argc, char **argv)
{
    int opt;

    bool silent = false;
    int errorCount = 0;

    while (-1 != (opt = getopt_long(argc, argv, "se:", _options, NULL))) {
        switch (opt) {
        case 's':
            silent = true;
            break;
        case 'e':
            errorCount = atoi(optarg);
            break;
        case '?':
            return EXIT_FAILURE;
        }
    }

    ParseInfo *info = NULL;
    NAIterator *iterator;

    SequenceBuilder *builder = SequenceBuilderCreate();
    Parser *parser = ParserCreate(builder);
    NAArray *events = ParserParseFile(parser, argv[optind], &info);
    ParserDestroy(parser);

    if (!silent) {
        printf("---- filenames ----\n");
        NAArraySort(info->filepaths, (void *)strcmp);
        iterator = NAArrayGetIterator(info->filepaths);
        while (iterator->hasNext(iterator)) {
            printf("%s\n", NAIOGetLastPathComponent(iterator->next(iterator)));
        }

        printf("\n");
        printf("---- events ----\n");
        NAArraySort(events, __MidiEventComparator);
        iterator = NAArrayGetIterator(events);
        while (iterator->hasNext(iterator)) {
            MidiEvent *event = iterator->next(iterator);
            __MidiEventDump(event);
        }

        printf("\n");
        printf("---- errors ----\n");
        NAArraySort(info->errors, __ParseErrorComparator);
        iterator = NAArrayGetIterator(info->errors);
        while (iterator->hasNext(iterator)) {
            ParseError *error = iterator->next(iterator);
            printf("[ERROR:%d] %s at %s:%d:%d",
                    error->code,
                    ParseErrorCode2String(error->code),
                    error->location.filepath,
                    error->location.line,
                    error->location.column);

            NAIterator *iterator2 = NAArrayGetIterator(error->infos);
            while (iterator2->hasNext(iterator2)) {
                printf(" %s", iterator2->next(iterator2));
            }

            printf("\n");
        }
    }

    bool success = errorCount == NAArrayCount(info->errors);

    ParseInfoRelease(info);
    builder->destroy(builder);

    bool leaked = NAAllocIsAllocatedMemoryExist();

    return success && !leaked ? EXIT_SUCCESS : EXIT_FAILURE;
}


static void SequenceBuilderDestroy(void *_self)
{
    SequenceBuilderImpl *self = _self;
    NAArrayTraverse(self->events, free);
    NAArrayDestroy(self->events);
    free(self);
}

static void SequenceBuilderSetResolution(void *_self, int resolution)
{
    SequenceBuilderImpl *self = _self;
    self->resolution = resolution;
}

static void SequenceBuilderAppendTitle(void *_self, int tick, const char *title)
{
    SequenceBuilderImpl *self = _self;
    TitleEvent *titleEvent = MidiEventAlloc(MidiEventTypeTitle, ++self->id, tick, strlen(title) + 1);
    strcpy(titleEvent->text, title);
    NAArrayAppend(self->events, titleEvent);
}

static void SequenceBuilderAppendCopyright(void *_self, int tick, const char *text)
{
    SequenceBuilderImpl *self = _self;
    CopyrightEvent *copyrightEvent = MidiEventAlloc(MidiEventTypeCopyright, ++self->id, tick, strlen(text) + 1);
    strcpy(copyrightEvent->text, text);
    NAArrayAppend(self->events, copyrightEvent);
}

static void SequenceBuilderAppendTempo(void *_self, int tick, float tempo)
{
    SequenceBuilderImpl *self = _self;
    TempoEvent *event = MidiEventAlloc(MidiEventTypeTempo, ++self->id, tick, sizeof(TempoEvent) - sizeof(MidiEvent));
    event->tempo = tempo;
    NAArrayAppend(self->events, event);
}

static void SequenceBuilderAppendTimeSign(void *_self, int tick, int numerator, int denominator)
{
    SequenceBuilderImpl *self = _self;
    TimeEvent *event = MidiEventAlloc(MidiEventTypeTime, ++self->id, tick, sizeof(TimeEvent) - sizeof(MidiEvent));
    event->numerator = numerator;
    event->denominator = denominator;
    NAArrayAppend(self->events, event);
}

static void SequenceBuilderAppendKey(void *_self, int tick, int sf, int mi)
{
    SequenceBuilderImpl *self = _self;
    KeyEvent *event = MidiEventAlloc(MidiEventTypeKey, ++self->id, tick, sizeof(KeyEvent) - sizeof(MidiEvent));
    event->sf = sf;
    event->mi = mi;
    NAArrayAppend(self->events, event);
}

static void SequenceBuilderAppendNote(void *_self, int tick, int channel, int noteNo, int gatetime, int velocity)
{
    SequenceBuilderImpl *self = _self;
    NoteEvent *event = MidiEventAlloc(MidiEventTypeNote, ++self->id, tick, sizeof(NoteEvent) - sizeof(MidiEvent));
    event->noteNo = noteNo;
    event->channel = channel;
    event->gatetime = gatetime;
    event->velocity = velocity;
    NAArrayAppend(self->events, event);
}

static void SequenceBuilderAppendMarker(void *_self, int tick, const char *marker)
{
    SequenceBuilderImpl *self = _self;
    MarkerEvent *event = MidiEventAlloc(MidiEventTypeMarker, ++self->id, tick, strlen(marker) + 1);
    strcpy(event->text, marker);
    NAArrayAppend(self->events, event);
}

static void SequenceBuilderAppendVoice(void *_self, int tick, int channel, int msb, int lsb, int programNo)
{
    SequenceBuilderImpl *self = _self;
    VoiceEvent *event = MidiEventAlloc(MidiEventTypeVoice, ++self->id, tick, sizeof(VoiceEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->msb = msb;
    event->lsb = lsb;
    event->programNo = programNo;
    NAArrayAppend(self->events, event);
}

static void SequenceBuilderAppendSynth(void *_self, int tick, int channel, const char *identifier)
{
    SequenceBuilderImpl *self = _self;
    SynthEvent *event = MidiEventAlloc(MidiEventTypeSynth, ++self->id, tick, strlen(identifier) + 1);
    event->channel = channel;
    strcpy(event->identifier, identifier);
    NAArrayAppend(self->events, event);
}

static void SequenceBuilderAppendVolume(void *_self, int tick, int channel, int value)
{
    SequenceBuilderImpl *self = _self;
    VolumeEvent *event = MidiEventAlloc(MidiEventTypeVolume, ++self->id, tick, sizeof(VolumeEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(self->events, event);
}

static void SequenceBuilderAppendPan(void *_self, int tick, int channel, int value)
{
    SequenceBuilderImpl *self = _self;
    PanEvent *event = MidiEventAlloc(MidiEventTypePan, ++self->id, tick, sizeof(PanEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(self->events, event);
}

static void SequenceBuilderAppendChorus(void *_self, int tick, int channel, int value)
{
    SequenceBuilderImpl *self = _self;
    ChorusEvent *event = MidiEventAlloc(MidiEventTypeChorus, ++self->id, tick, sizeof(ChorusEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(self->events, event);
}

static void SequenceBuilderAppendReverb(void *_self, int tick, int channel, int value)
{
    SequenceBuilderImpl *self = _self;
    ReverbEvent *event = MidiEventAlloc(MidiEventTypeReverb, ++self->id, tick, sizeof(ReverbEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(self->events, event);
}

static void SequenceBuilderAppendExpression(void *_self, int tick, int channel, int value)
{
    SequenceBuilderImpl *self = _self;
    ExpressionEvent *event = MidiEventAlloc(MidiEventTypeExpression, ++self->id, tick, sizeof(ExpressionEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(self->events, event);
}

static void SequenceBuilderAppendDetune(void *_self, int tick, int channel, int value)
{
    SequenceBuilderImpl *self = _self;
    DetuneEvent *event = MidiEventAlloc(MidiEventTypeDetune, ++self->id, tick, sizeof(DetuneEvent) - sizeof(MidiEvent));
    event->channel = channel;
    event->value = value;
    NAArrayAppend(self->events, event);
}

static void SequenceBuilderSetLength(void *_self, int length)
{
    SequenceBuilderImpl *self = _self;
    self->length = length;
}

static int __MidiEventComparator(const void *_event1, const void *_event2)
{
    const MidiEvent *event1 = *((const MidiEvent **)_event1);
    const MidiEvent *event2 = *((const MidiEvent **)_event2);

    int result;
    
    result = MidiEventGetChannel(event1) - MidiEventGetChannel(event2);
    if (0 != result) {
        return result;
    }

    result = event1->tick - event2->tick;
    if (0 != result) {
        return result;
    }

    return event1->id - event2->id;
}


static void *SequenceBuilderBuild(void *_self)
{
    SequenceBuilderImpl *self = _self;
    return self->events;
}

SequenceBuilder *SequenceBuilderCreate()
{
    SequenceBuilderImpl *self = calloc(1, sizeof(SequenceBuilderImpl));

    self->interface.destroy = SequenceBuilderDestroy;
    self->interface.setResolution = SequenceBuilderSetResolution;
    self->interface.appendTitle = SequenceBuilderAppendTitle;
    self->interface.appendCopyright = SequenceBuilderAppendCopyright;
    self->interface.appendTempo = SequenceBuilderAppendTempo;
    self->interface.appendTimeSign = SequenceBuilderAppendTimeSign;
    self->interface.appendKey = SequenceBuilderAppendKey;
    self->interface.appendNote = SequenceBuilderAppendNote;
    self->interface.appendMarker = SequenceBuilderAppendMarker;
    self->interface.appendVoice = SequenceBuilderAppendVoice;
    self->interface.appendSynth = SequenceBuilderAppendSynth;
    self->interface.appendVolume = SequenceBuilderAppendVolume;
    self->interface.appendPan = SequenceBuilderAppendPan;
    self->interface.appendChorus = SequenceBuilderAppendChorus;
    self->interface.appendReverb = SequenceBuilderAppendReverb;
    self->interface.appendExpression = SequenceBuilderAppendExpression;
    self->interface.appendDetune = SequenceBuilderAppendDetune;
    self->interface.setLength = SequenceBuilderSetLength;
    self->interface.build = SequenceBuilderBuild;

    self->events = NAArrayCreate(64, NADescriptionAddress);

    return (SequenceBuilder *)self;
}

static void __MidiEventDump(MidiEvent *self)
{
    void *_self = self;
    
    switch (self->type) {
    case MidiEventTypeNote:
        {
            NoteEvent *self = _self;
            printf("Note: tick=%d channel=%d noteNo=%d gatetime=%d velocity=%d\n", self->tick, self->channel, self->noteNo, self->gatetime, self->velocity);
        }
        break;
    case MidiEventTypeTempo:
        {
            TempoEvent *self = _self;
            printf("Tempo: tick=%d tempo=%f\n", self->tick, self->tempo);
        }
        break;
    case MidiEventTypeTime:
        {
            TimeEvent *self = _self;
            printf("Time: tick=%d time=%d/%d\n", self->tick, self->numerator, self->denominator);
        }
        break;
    case MidiEventTypeKey:
        {
            KeyEvent *self = _self;
            printf("Key: tick=%d sf=%d mi=%d\n", self->tick, self->sf, self->mi);
        }
        break;
    case MidiEventTypeTitle:
        {
            TitleEvent *self = _self;
            printf("Title: tick=%d text=%s\n",
                    self->tick, self->text);
        }
        break;
    case MidiEventTypeCopyright:
        {
            CopyrightEvent *self = _self;
            printf("Copyright: tick=%d text=%s\n", self->tick, self->text);
        }
        break;
    case MidiEventTypeMarker:
        {
            MarkerEvent *self = _self;
            printf("Marker: tick=%d text=%s\n", self->tick, self->text);
        }
        break;
    case MidiEventTypeVoice:
        {
            VoiceEvent *self = _self;
            printf("Voice: tick=%d channel=%d msb=%d lsb=%d programNo=%d\n", self->tick, self->channel, self->msb, self->lsb, self->programNo);
        }
        break;
    case MidiEventTypeVolume:
        {
            VolumeEvent *self = _self;
            printf("Volume: tick=%d channel=%d value=%d\n", self->tick, self->channel, self->value);
        }
        break;
    case MidiEventTypePan:
        {
            PanEvent *self = _self;
            printf("Pan: tick=%d channel=%d value=%d\n", self->tick, self->channel, self->value);
        }
        break;
    case MidiEventTypeChorus:
        {
            ChorusEvent *self = _self;
            printf("Chorus: tick=%d channel=%d value=%d\n", self->tick, self->channel, self->value);
        }
        break;
    case MidiEventTypeReverb:
        {
            ReverbEvent *self = _self;
            printf("Reverb: tick=%d channel=%d value=%d\n", self->tick, self->channel, self->value);
        }
        break;
    case MidiEventTypeExpression:
        {
            ExpressionEvent *self = _self;
            printf("Expression: tick=%d value=%d\n", self->tick, self->value);
        }
        break;
    case MidiEventTypeDetune:
        {
            DetuneEvent *self = _self;
            printf("Detune: tick=%d value=%d\n", self->tick, self->value);
        }
        break;
    case MidiEventTypeSynth:
        {
            SynthEvent *self = _self;
            printf("Synth: tick=%d identifier=%s\n", self->tick, self->identifier);
        }
        break;
    }
}

static int __ParseErrorComparator(const void *_error1, const void *_error2)
{
    const ParseError *error1 = *((const ParseError **)_error1);
    const ParseError *error2 = *((const ParseError **)_error2);

    int result;
    
    result = strcmp(error1->location.filepath, error2->location.filepath);
    if (0 != result) {
        return result;
    }

    result = error1->location.line - error2->location.line;
    if (0 != result) {
        return result;
    }

    result = error1->location.column - error2->location.column;
    if (0 != result) {
        return result;
    }

    return error1->code - error2->code;
}
