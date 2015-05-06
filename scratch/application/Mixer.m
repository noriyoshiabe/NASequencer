#import "Mixer.h"

@interface Mixer()
@property (nonatomic, strong) NSMutableDictionary *clients;
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
}

- (void)sendNoteOn:(NoteEvent *)event
{
    printf("Mixer sendNoteOn:\n");
}

- (void)sendNoteOff:(NoteEvent *)event
{
    printf("Mixer sendNoteOff:\n");
}

- (void)sendAllNoteOff
{
    printf("Mixer sendAllNoteOff:\n");
}

@end
