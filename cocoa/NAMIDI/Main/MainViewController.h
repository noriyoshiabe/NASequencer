//
//  MainViewController.h
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Stub.h"
#import "TrackSelection.h"

@interface MainViewController : NSViewController
@property (strong, nonatomic) NAMidiRepresentation *namidi;
@property (strong, nonatomic) TrackSelection *trackSelection;
@end
