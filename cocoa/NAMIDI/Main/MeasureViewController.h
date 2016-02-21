//
//  MeasureViewController.h
//  NAMIDI
//
//  Created by abechan on 2/17/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MeasureScaleAssistant.h"
#import "TrackSelection.h"
#import "Stub.h"

@interface MeasureViewController : NSViewController
@property (strong, nonatomic) NAMidiRepresentation *namidi;
@property (strong, nonatomic) MeasureScaleAssistant *scaleAssistant;
@property (strong, nonatomic) TrackSelection *trackSelection;
@end
