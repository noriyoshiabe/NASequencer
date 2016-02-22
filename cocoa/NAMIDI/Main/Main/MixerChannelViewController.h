//
//  MixerChannelViewController.h
//  NAMIDI
//
//  Created by abechan on 2/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Stub.h"

@interface MixerChannelViewController : NSViewController
@property (assign, nonatomic) int channel;
@property (strong, nonatomic) NAMidiRepresentation *namidi;
@end
