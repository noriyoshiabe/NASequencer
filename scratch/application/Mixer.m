#import "Mixer.h"
#import "MidiClient.h"
#import "CoreMidiClient.h"

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

@end
