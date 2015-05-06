#pragma once

#import <Foundation/Foundation.h>

#import "MidiEvent.h"

@interface Mixer : NSObject
+ (Mixer *)sharedInstance;
- (void)initialize;
- (void)sendNoteOn:(NoteEvent *)event;
- (void)sendNoteOff:(NoteEvent *)event;
- (void)sendAllNoteOff;
@end
