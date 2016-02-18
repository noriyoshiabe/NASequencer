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
@property (assign, nonatomic) bool mute;
@property (assign, nonatomic) bool solo;
@property (assign, nonatomic) int volume;
@property (assign, nonatomic) int pan;
@property (assign, nonatomic) int chorus;
@property (assign, nonatomic) int reverb;
@property (assign, nonatomic) int L;
@property (assign, nonatomic) int R;

@property (readonly) NSMutableArray *midiSources;
@property (readonly) NSArray *presets;

@property (strong, nonatomic) MidiSourceRepresentation *midiSource;
@property (strong, nonatomic) PresetRepresentation *preset;

@end
