//
//  MidiSourceManagerRepresentation.h
//  NAMIDI
//
//  Created by abechan on 2/26/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface MidiSourceManagerRepresentation : NSObject
+ (MidiSourceManagerRepresentation *)sharedInstance;
- (void)loadMidiSourceDescriptionFromSoundFont:(NSString *)filepath;
@end
