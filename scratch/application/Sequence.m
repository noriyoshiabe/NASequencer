#import "Sequence.h"
#import "SMFWriter.h"

@interface Sequence() {
    TimeTable *_timeTable;
}

@property (nonatomic, readwrite) NSArray *events;

@end

@interface NoteEvent(SMF)
@property (nonatomic, readonly) BOOL isNoteOff;
@end

@interface NotoOffEvent : NoteEvent
@property (nonatomic, readonly) BOOL isNoteOff;
@end

@implementation Sequence

- (uint16_t)resolution
{
    return TimeTableResolution(_timeTable);
}

- (int32_t)length
{
    return TimeTableLength(_timeTable);
}

- (TimeSign)timeSign:(int32_t)tick
{
    return TimeTableTimeSignOnTick(_timeTable, tick);
}

- (Location)location:(int32_t)tick
{
    return TimeTableTick2Location(_timeTable, tick);
}

- (TimeTable *)timeTable
{
    return _timeTable;
}

- (id)initWithEvents:(NSArray *)events timeTable:(TimeTable *)timeTable
{
    if (self = [super init]) {
        self.events = events;
        _timeTable = TimeTableRetain(timeTable);
    }
    return self;
}

- (void)dealloc
{
    TimeTableRelease(_timeTable);
}

- (NSString *)description
{
    char *buffer = alloca(2048);
    TimeTableDumpToBuffer(_timeTable, buffer, 2048);
    return [NSString stringWithFormat:@"<%@\ntimeTable=%s\nevents=%@>", [self.class description], buffer, self.events];
}

- (BOOL)serialize:(NSString *)filepath
{
    SMFWriter *writer = SMFWriterCreate([filepath UTF8String]);

    NSMutableArray *withNoteOff = [self.events mutableCopy];
    for (MidiEvent *event in self.events) {
        if (MidiEventTypeNote == event.type) {
            NoteEvent *note = (NoteEvent *)event;
            NotoOffEvent *noteOff = [[NotoOffEvent alloc] initWithTick:note.offTick channel:note.channel noteNo:note.noteNo velocity:0 gatetime:0];
            [withNoteOff addObject:noteOff];
        }
    }

    NSArray *toWrite = [withNoteOff sortedArrayUsingSelector:@selector(compare:)];

    SMFWriterSetResolution(writer, self.resolution);

    for (MidiEvent *event in toWrite) {
        switch (event.type) {
        case MidiEventTypeNote:
            {
                NoteEvent *note = (NoteEvent *)event;
                if (note.isNoteOff) {
                    SMFWriterAppendNoteOff(writer, note.tick, note.channel, note.noteNo, note.velocity);
                } else {
                    SMFWriterAppendNoteOn(writer, note.tick, note.channel, note.noteNo, note.velocity);
                }
            }
            break;
        case MidiEventTypeTime:
            {
                TimeEvent *time = (TimeEvent *)event;
                SMFWriterAppendTime(writer, time.tick, time.numerator, time.denominator);
            }
            break;
        case MidiEventTypeTempo:
            {
                TempoEvent *tempo = (TempoEvent *)event;
                SMFWriterAppendTempo(writer, tempo.tick, tempo.tempo);
            }
            break;
        case MidiEventTypeMarker:
            {
                MarkerEvent *marker = (MarkerEvent *)event;
                SMFWriterAppendMarker(writer, marker.tick, [marker.text UTF8String]);
            }
            break;
        case MidiEventTypeSound:
            {
                SoundEvent *sound = (SoundEvent *)event;
                SMFWriterAppendSound(writer, sound.tick, sound.channel, sound.msb, sound.lsb, sound.programNo);
            }
            break;
        }
    }

    BOOL ret = SMFWriterSerialize(writer);
    SMFWriterDestroy(writer);
    return ret;
}

@end

@implementation NoteEvent(SMF)
- (BOOL)isNoteOff { return NO; }
@end

@implementation NotoOffEvent
- (BOOL)isNoteOff { return YES; }
@end
