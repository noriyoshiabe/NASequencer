#import "Mixer.h"

@interface MidiSourceRepresentation() {
    NSMutableArray *_presets;
    MidiSourceDescription *_description;

@public
    MidiSource *native;
}

@end

@interface PresetRepresentation()

@property (nonatomic, strong, readwrite) NSString *name;
@property (nonatomic, readwrite) uint16_t bankNo;
@property (nonatomic, readwrite) uint8_t programNo;

@end

@interface ChannelRepresentation()
@property (nonatomic, readwrite) uint8_t number;
@property (nonatomic, weak) Mixer *mixer;
@property (nonatomic) bool active;
@end

@interface Mixer() {
    NSMutableArray *_midiSources;
    NSMutableArray *_channels;
}

- (void)updateActiveChannels;

@end


@implementation MidiSourceRepresentation

@synthesize description = _description;

- (id)initWithMidiSource:(MidiSource *)midiSource description:(MidiSourceDescription *)description
{
    if (self = [super init]) {
        native = midiSource;
        
        _description = description;
        _presets = [NSMutableArray array];

        int presetCount = native->getPresetCount(native);
        PresetList presetList[presetCount];
        native->getPresetList(native, presetList);

        for (int i = 0; i < presetCount; ++i) {
            PresetRepresentation *preset = [[PresetRepresentation alloc] init];
            preset.name = [NSString stringWithCString:presetList[i].name encoding:NSUTF8StringEncoding];
            preset.bankNo = presetList[i].bankNo;
            preset.programNo = presetList[i].programNo;
            [_presets addObject: preset];
        }
    }
    return self;
}

- (void)dealloc
{
    native->destroy(native);
}

- (Level)level
{
    return native->getMasterLevel(native);
}

@end


@implementation ChannelRepresentation

- (Level)level
{
    return _midiSource->native->getChannelLevel(_midiSource->native, _number);
}

- (void)setMidiSource:(MidiSourceRepresentation *)midiSource
{
    if (_midiSource != midiSource) {
        _midiSource = midiSource;
        self.preset = [_midiSource.presets firstObject];
    }
}

- (void)setPreset:(PresetRepresentation *)preset
{
    _preset = preset;
    _midiSource->native->setPresetIndex(_midiSource->native, _number, [_midiSource.presets indexOfObject:_preset]);
}

- (void)setVolume:(uint8_t)volume;
{
    _volume = volume;
    _midiSource->native->setVolume(_midiSource->native, _number, volume);
}

- (void)setPan:(uint8_t)pan;
{
    _pan = pan;
    _midiSource->native->setPan(_midiSource->native, _number, pan);
}

- (void)setChorusSend:(uint8_t)chorusSend;
{
    _chorusSend = chorusSend;
    _midiSource->native->setChorusSend(_midiSource->native, _number, chorusSend);
}

- (void)setReverbSend:(uint8_t)reverbSend;
{
    _reverbSend = reverbSend;
    _midiSource->native->setReverbSend(_midiSource->native, _number, reverbSend);
}

- (void)setMute:(bool)mute;
{
    _mute = mute;
    [_mixer updateActiveChannels];
}

- (void)setSolo:(bool)solo;
{
    _solo = solo;
    [_mixer updateActiveChannels];
}

@end


@implementation Mixer

- (id)init
{
    if (self = [super init]) {
        _midiSources = [NSMutableArray array];
        _channels = [NSMutableArray array];

        for (MidiSourceDescription *description in [MidiSourceManager sharedInstance].descriptions) {
            if (description.available) {
                MidiSource *midiSource = [[MidiSourceManager sharedInstance] createMidiSource:description];
                [_midiSources addObject:[[MidiSourceRepresentation alloc] initWithMidiSource:midiSource description:description]];
            }
        }

        for (int i = 0; i < 16; ++i) {
            ChannelRepresentation *channel = [[ChannelRepresentation alloc] init];
            channel.number = i;

            channel.midiSource = [_midiSources firstObject];
            channel.preset = [channel.midiSource.presets firstObject];
            channel.volume = 100;
            channel.pan = 64;
            channel.chorusSend = 0;
            channel.reverbSend = 0;

            channel.mute = false;
            channel.solo = false;
            channel.active = true;

            channel.mixer = self;

            [_channels addObject:channel];
        }
    }
    return self;
}

- (Level)level
{
    return (Level){0,0}; // TODO
}

- (void)sendNoteOn:(NoteEvent *)event
{
    uint8_t bytes[3] = {0x90 | (0x0F & (event.channel - 1)), event.noteNo, event.velocity};

    ChannelRepresentation *channel = _channels[event.channel - 1];
    if (channel.active) {
        MidiSource *midiSource = channel.midiSource->native;
        midiSource->send(midiSource, bytes, sizeof(bytes));
    }
}

- (void)sendNoteOff:(NoteEvent *)event
{
    uint8_t bytes[3] = {0x80 | (0x0F & (event.channel - 1)), event.noteNo, 0x00};

    ChannelRepresentation *channel = _channels[event.channel - 1];
    if (channel.active) {
        MidiSource *midiSource = channel.midiSource->native;
        midiSource->send(midiSource, bytes, sizeof(bytes));
    }
}

- (void)sendAllNoteOff
{
    uint8_t bytes[3] = {0, 0x7B, 0x00};
    for(int i = 0; i < 16; ++i) {
        ChannelRepresentation *channel = _channels[i];
        MidiSource *midiSource = channel.midiSource->native;
        bytes[0] = 0xB0 | (0x0F & i);
        midiSource->send(midiSource, bytes, sizeof(bytes));
    }
}

- (void)sendSound:(SoundEvent *)event
{
    uint8_t bytes[3];

    ChannelRepresentation *channel = _channels[event.channel - 1];
    MidiSource *midiSource = channel.midiSource->native;

    bytes[0] = 0xB0 | (0x0F & (event.channel - 1));
    bytes[1] = 0x00;
    bytes[2] = event.msb;
    midiSource->send(midiSource, bytes, sizeof(bytes));

    bytes[1] = 0x20;
    bytes[2] = event.lsb;
    midiSource->send(midiSource, bytes, sizeof(bytes));

    bytes[0] = 0xC0 | (0x0F & (event.channel - 1));
    bytes[1] = event.programNo;
    midiSource->send(midiSource, bytes, sizeof(bytes));
}

- (void)addObserver:(id<MixerObserver>)observer
{
    // TODO
}

- (void)removeObserver:(id<MixerObserver>)observer
{
    // TODO
}

- (void)updateActiveChannels
{
    // TODO
}

@end
