#import "Mixer.h"

@interface MidiSourceRepresentation() {
    MidiSource *midiSource;
}

@property (nonatomic, strong, readwrite) MidiSourceDescription *description;
@property (nonatomic, strong, readwrite) NSMutableArray *presets;

@end

@interface PresetRepresentation()

@property (nonatomic, strong, readwrite) NSString *name;
@property (nonatomic, readwrite) uint16_t bankNo;
@property (nonatomic, readwrite) uint8_t programNo;

@end

@implementation MidiSourceRepresentation

- (id)initWithMidiSource:(MidiSource *)midiSource description:(MidiSourceDescription *)description
{
    if (self = [super init]) {
        self->midiSource = midiSource;
        
        self.description = description;
        self.presets = [NSMutableArray array];

        int presetCount = midiSource->getPresetCount(midiSource);
        PresetList plisetList[presetCount];
        midiSource->getPresetList(midiSource, presetList);

        for (int i = 0; i < presetCount; ++i) {
            PresetRepresentation *preset = [[PresetRepresentation alloc] init];
            preset.name = [NSString stringWithCString:plisetList[i].name encoding:NSUTF8StringEncoding];
            preset.bankNo = plisetList[i].bankNo;
            preset.programNo = plisetList[i].programNo;
            [presets addObject: preset];
        }
    }
    return self;
}

- (void)dealloc
{
    self.description = nil;
    self.presets = nil;
    midiSource->destroy(midiSource);
}

@end


@interface Mixer() {
    MidiClient **clients;
}
@end

@implementation Mixer

static Mixer *_sharedInstance = nil;

+ (Mixer *)sharedInstance
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _sharedInstance = [[Mixer alloc] init];
    });
    return _sharedInstance;
}

- (void)initialize
{
    clients = calloc(16, sizeof(MidiClient *));
    for (int i = 0; i < 16; ++i) {
        clients[i] = CoreMidiClientSharedInstance();
    }
}

- (void)sendNoteOn:(NoteEvent *)event
{
    uint8_t bytes[3] = {0x90 | (0x0F & (event.channel - 1)), event.noteNo, event.velocity};

    MidiClient *client = clients[event.channel - 1];
    client->send(client, bytes, sizeof(bytes));
}

- (void)sendNoteOff:(NoteEvent *)event
{
    uint8_t bytes[3] = {0x80 | (0x0F & (event.channel - 1)), event.noteNo, 0x00};

    MidiClient *client = clients[event.channel - 1];
    client->send(client, bytes, sizeof(bytes));
}

- (void)sendAllNoteOff
{
    uint8_t bytes[3] = {0, 0x7B, 0x00};
    for(int i = 0; i < 16; ++i) {
        MidiClient *client = clients[i];
        bytes[0] = 0xB0 | (0x0F & i);
        client->send(client, bytes, sizeof(bytes));
    }
}

- (void)sendSound:(SoundEvent *)event
{
    uint8_t bytes[3];
    MidiClient *client = clients[event.channel - 1];

    bytes[0] = 0xB0 | (0x0F & (event.channel - 1));
    bytes[1] = 0x00;
    bytes[2] = event.msb;
    client->send(client, bytes, sizeof(bytes));

    bytes[1] = 0x20;
    bytes[2] = event.lsb;
    client->send(client, bytes, sizeof(bytes));

    bytes[0] = 0xC0 | (0x0F & (event.channel - 1));
    bytes[1] = event.programNo;
    client->send(client, bytes, sizeof(bytes));
}

@end
