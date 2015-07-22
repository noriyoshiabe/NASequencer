#import "MidiSourceManager.h"
#import "SoundFont.h"
#import "AudioOut.h"

@interface MidiSourceDescription() {
    SoundFont *soundFont;
}

@property (nonatomic, strong, readwrite) NSString *name;
@property (nonatomic, strong, readwrite) NSString *filepath;
@property (nonatomic, readwrite) bool available;
@property (nonatomic, readwrite) MidiSourceDescriptionError error;

@end

@implementation MidiSourceDescription

- (void)dealloc
{
    if (soundFont) {
        SoundFontDestroy(soundFont);
    }

    self.filepath = nil;
    self.name = nil;
}

@end

@interface MidiSourceManager()
@property (nonatomic, strong, readwrite) NSMutableArray *descriptions;
@end

@implementation MidiSourceManager

static MidiSourceManager *_sharedInstance = nil;

+ (MidiSourceManager *)sharedInstance
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _sharedInstance = [[MidiSourceManager alloc] init];
        [_sharedInstance initialize];
    });
    return _sharedInstance;
}

- (void)initialize
{
    self.descriptions = [NSMutableArray array];
}

- (void)loadSoundFont:(NSString *)filepath
{
    SoundFontError error;
    SoundFont *soundFont = SoundFontRead([filepath UTF8String], &error);

    MidiSourceDescription *description = [[MidiSourceDescription alloc] init];
    description.soundFont = soundFont;
    description.filepath = filepath;

    if (soundFont) {
        description.name = [NSString stringWithCString:soundFont->INAM encoding:NSUTF8StringEncoding];
        description.available = true;
    }
    else {
        description.name = [[filepath lastPathComponent] stringByDeletingPathExtension];

        switch (error) {
        case SoundFontErrorFileNotFound:
            description.error = MidiSourceDescriptionErrorFileNotFound;
            break;
        case SoundFontErrorUnsupportedVersion:
            description.error = MidiSourceDescriptionErrorUnsupportedVersion;
            break;
        case SoundFontErrorInvalidFileFormat:
            description.error = MidiSourceDescriptionErrorInvalidFileFormat;
            break;
        }
    }

    [descriptions addObject:description];
}

- (void)unloadMidiSource:(MidiSourceDescription *)description
{
    // TODO
}

- (MidiSource *)createMidiSource:(MidiSourceDescription *)description
{
    if (!description.soundFont) {
        return NULL;
    }

    AudioOut *audioOut = AudioOutSharedInstance();
    return (MidiSource *)SynthesizerCreate(description.soundFont, AudioOutGetSampleRate(audioOut));
}

- (void)addObserver:(id<MidiSourceManagerObserver>)observer
{
    // TODO
}

- (void)removeObserver:(id<MidiSourceManagerObserver>)observer
{
    // TODO
}

@end
