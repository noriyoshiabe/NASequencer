//
//  MixerChannelViewController.h
//  NAMIDI
//
//  Created by abechan on 2/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MixerRepresentation.h"

@interface MixerChannelViewController : NSViewController
@property (strong, nonatomic) MixerRepresentation *mixer;
@property (assign, nonatomic) int channel;
@end
