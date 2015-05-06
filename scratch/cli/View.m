#import "View.h"

@implementation View

- (void)setNamidi:(NAMidi *)namidi
{
    _namidi = namidi;
    [_namidi addObserver:self];
}

- (void)namidi:(NAMidi *)namidi onParseFinish:(Sequence *)sequence
{
    NSLog(@"onParseFinish: %@", sequence);
}

- (void)namidi:(NAMidi *)namidi onParseError:(NSString *)filepath line:(int)line column:(int)column error:(ParseError)error info:(const void *)info
{
    NSLog(@"onParseError: filepath=%@ line=%d column=%d error=%s\n", filepath, line, column, ParseError2String(error));
}

- (void)namidi:(NAMidi *)namidi player:(Player *)player notifyEvent:(PlayerEvent)playerEvent
{
    NSLog(@"player:notifyEvent: playerEvent=%@\n", [Player playerEvent2String:playerEvent]);
}

- (void)namidi:(NAMidi *)namidi player:(Player *)player didSendNoteOn:(NoteEvent *)noteEvent
{
    NSLog(@"player:didSendNoteOn: noteEvent=%@\n", noteEvent);
}

- (void)namidi:(NAMidi *)namidi player:(Player *)player didSendNoteOff:(NoteEvent *)noteEvent
{
    NSLog(@"player:didSendNoteOff: noteEvent=%@\n", noteEvent);
}

@end

