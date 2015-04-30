// C layer

typedef struct _NAMidiParser NAMidiParser;

typedef struct {
    void (*onParseResolution)(uint16_t resolution);
    void (*onParseTitle)(const char *text);
    void (*onParseNote)(uint32_t tick, uint8_t channel, uint8_t noteNo, uint8_t velocity, uint32_t gatetime);
    void (*onParseTime)(uint32_t tick, uint8_t numerator, uint8_t denominator);
    void (*onParseTempo)(uint32_t tick, float tempo);
    void (*onParseMarker)(uint32_t tick, const char *text);
    void (*onParseSoundSelect)(uint32_t tick, uint8_t channel, uint8_t msb, uint8_t lsb, uint8_t programNo);

    void (*onFinish)(uint32_t length);
    void (*onError)(const char *filepath, int line, int column, NAMidiParserError error, ...);
} NAMidiParserCallbacks;

extern NAMidiParser *NAMidiParserCreate(NAMidiParserCallbacks *callbacks);
extern void NAMidiParserExecuteParse(NAMidiParser *self);
extern void NAMidiParserDestroy(NAMidiParser *self);



typedef struct _PlayerClockSource PlayerClockSource;

typedef struct _PlayerClockSourceCallbacks {
    void (*onSupplyClock)(int64_t usec, uint32_t tick, const PlayerLocation *location);
    void (*onStateChange)(PlayerState state);

    void (*tempoOnTick)(uint32_t tick, float *tempo);
    void (*timeSignOnTick)(uint32_t tick, uint8_t *numerator, uint8_t *denominator);
    int64_t (*tick2MicroSec)(uint32_t tick);
    uint32_t (*microSec2Tick)(int64_t usec);
    uint32_t (*location2Tick)(const PlayerLocation *location);
} PlayerClockSourceCallbacks;

extern PlayerClockSource *PlayerClockSourceCreate(PlayerClockSourceCallbacks *callbacks);
extern void PlayerClockSourceDestroy(PlayerClockSource *self);


typedef struct _FSWatcher FSWatcher;

typedef struct _FSWatcherCallbacks {
    void (*onFileChanged)(const char *changedFile);
    void (*onError)(int error, const char *message);
} FSWatcherCallbacks;

extern FSWatcher *FSWatcherCreate(FSWatcherCallbacks *callbacks);

extern void FSWatcherRegisterFilepath(FSWatcher *self, CFStringRef filepath);
extern void FSWatcherStart(FSWatcher *self);
extern void FSWatcherDestroy(FSWatcher *self);


typedef struct _MidiClient MidiClient;

extern MidiClient *MidiClientCreate(MidiSource *midiSource);
extern void MidiClientSend(MidiClient *self, uint8_t *bytes, size_t length);
extern void MidiClientDestroy(MidiClient *self);


extern MidiSource *CoreMidiClientCreate();
//extern MidiSource *SoundFontEngineCreate(const char *sf2Path);

extern void MidiSourceSend(MidiSource *self, uint8_t *bytes, size_t length);
extern bool MidiSourceIsAvailable(MidiSource *self);
extern void MidiSourceSetVolume(MidiSource *self);
// extern void MidiSourceSetPan(MidiSource *self);
extern void MidiSourceDestroy(MidiSource *self);


// Obj-C layer
ApplicationModel;
Player;
Mixer;
Sequence;

// Swift layer
View View View!!;
