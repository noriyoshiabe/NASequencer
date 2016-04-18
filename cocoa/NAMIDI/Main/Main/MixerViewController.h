//
//  MixerViewController.h
//  NAMIDI
//
//  Created by abechan on 2/17/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "NAMidiRepresentation.h"
#import "MixerRepresentation.h"

@class MixerViewController;
@protocol MixerViewControllerDelegate <NSObject>
- (void)mixerViewController:(MixerViewController *)controller didSelectPresetButtonWithChannel:(MixerChannelRepresentation *)mixerChannel;
@end

@interface MixerViewController : NSViewController
@property (assign, nonatomic) id<MixerViewControllerDelegate> delegate;
@property (strong, nonatomic) NAMidiRepresentation *namidi;
@end
