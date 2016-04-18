//
//  PresetSelectionWindowController.h
//  NAMIDI
//
//  Created by abechan on 4/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MixerRepresentation.h"

@interface PresetSelectionWindowController : NSWindowController
@property (strong, nonatomic) MixerRepresentation *mixer;
@property (strong, nonatomic) MixerChannelRepresentation *mixerChannel;
@end
