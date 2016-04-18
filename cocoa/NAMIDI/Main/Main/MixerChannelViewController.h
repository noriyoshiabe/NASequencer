//
//  MixerChannelViewController.h
//  NAMIDI
//
//  Created by abechan on 2/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MixerRepresentation.h"

@class MixerChannelViewController;
@protocol MixerChannelViewControllerDelegate <NSObject>
- (void)mixerChannelViewController:(MixerChannelViewController *)controller didSelectChannel:(MixerChannelRepresentation *)mixerChannel;
@end

@interface MixerChannelViewController : NSViewController
@property (assign, nonatomic) id<MixerChannelViewControllerDelegate> delegate;
@property (strong, nonatomic) MixerRepresentation *mixer;
@property (assign, nonatomic) int channel;
@end
