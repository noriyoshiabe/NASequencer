//
//  PianoRollKeyboardViewController.h
//  NAMIDI
//
//  Created by abechan on 2/24/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "NAMidiRepresentation.h"
#import "TrackSelection.h"

@interface PianoRollKeyboardViewController : NSViewController
@property (strong, nonatomic) NAMidiRepresentation *namidi;
@property (strong, nonatomic) TrackSelection *trackSelection;
@end
