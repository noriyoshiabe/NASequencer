//
//  SelectionViewController.h
//  NAMIDI
//
//  Created by abechan on 2/21/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TrackSelection.h"

@interface SelectionViewController : NSViewController
@property (strong, nonatomic) NAMidiRepresentation *namidi;
@property (strong, nonatomic) TrackSelection *trackSelection;
@end
