//
//  EventListViewController.h
//  NAMIDI
//
//  Created by abechan on 2/22/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TrackSelection.h"
#import "NAMidiRepresentation.h"

@interface EventListViewController : NSViewController
@property (strong, nonatomic) NAMidiRepresentation *namidi;
@property (strong, nonatomic) TrackSelection *trackSelection;
- (IBAction)filterNoteEvent:(id)sender;
- (IBAction)filterControlEvent:(id)sender;
@end
