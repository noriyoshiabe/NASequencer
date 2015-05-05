#pragma

#import <Foundation/Foundation.h>

#import "TimeTable.h"
#import "Sequence.h"
#import "MidiEvent.h"

typedef NS_ENUM (NSUInteger, PlayerState) {
    PlayerStateStop,
    PlayerStatePlaying,
};

@protocol PlayerDelegate;

@interface Player : NSObject

@property (nonatomic, readonly) int64_t usec;
@property (nonatomic, readonly) int32_t tick;
@property (nonatomic, readonly) Location location;

@property (nonatomic, strong) Sequence *sequence;
@property (nonatomic, weak) id<PlayerDelegate> delegate;

@end

@protocol PlayerDelegate <NSObject>
@optional
- (void)player:(Player *)player didChangeState:(PlayerState)state;
- (void)player:(Player *)player didSendNoteOn:(NoteEvent *)noteEvent;
- (void)player:(Player *)player didSendNoteOff:(NoteEvent *)noteEvent;
@end

