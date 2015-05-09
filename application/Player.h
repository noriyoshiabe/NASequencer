#pragma

#import <Foundation/Foundation.h>

#import "TimeTable.h"
#import "Sequence.h"
#import "MidiEvent.h"

typedef NS_ENUM (NSUInteger, PlayerEvent) {
    PlayerEventStop,
    PlayerEventPlay,
    PlayerEventRewind,
    PlayerEventForward,
    PlayerEventBackward,
    PlayerEventReachEnd,
};

@protocol PlayerDelegate;

@interface Player : NSObject

@property (nonatomic, readonly) bool playing;
@property (nonatomic, readonly) int64_t usec;
@property (nonatomic, readonly) int32_t tick;
@property (nonatomic, readonly) Location location;
@property (nonatomic, readonly) NSSet *playingNoteEvents;

@property (nonatomic, strong) Sequence *sequence;
@property (nonatomic, weak) id<PlayerDelegate> delegate;

- (void)stop;
- (void)play;
- (void)playPause;
- (void)rewind;
- (void)forward;
- (void)backward;

+ (NSString *)playerEvent2String:(PlayerEvent)playerEvent;

@end

@protocol PlayerDelegate <NSObject>
@optional
- (void)player:(Player *)player notifyEvent:(PlayerEvent)playerEvent;
- (void)player:(Player *)player onClock:(uint32_t)tick usec:(int64_t)usec location:(Location)location;
- (void)player:(Player *)player didSendNoteOn:(NoteEvent *)noteEvent;
- (void)player:(Player *)player didSendNoteOff:(NoteEvent *)noteEvent;
- (void)player:(Player *)player didSendSound:(SoundEvent *)soundEvent;
@end

