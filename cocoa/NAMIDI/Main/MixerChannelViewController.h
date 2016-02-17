//
//  MixerChannelViewController.h
//  NAMIDI
//
//  Created by abechan on 2/18/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface MixerChannelViewController : NSViewController
@property (assign, nonatomic) int channel;
@property (assign, nonatomic) bool mute;
@property (assign, nonatomic) bool solo;
@property (assign, nonatomic) int volume;
@property (assign, nonatomic) int pan;
@property (assign, nonatomic) int chorus;
@property (assign, nonatomic) int reverb;
//@property (readonly, nonatomic) NSArray *synthsizerIdentifiers;
//@property (strong, nonatomic) NSString *synthsizerIdentifier;
@end
