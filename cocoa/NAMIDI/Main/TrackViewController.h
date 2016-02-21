//
//  TrackViewController.h
//  NAMIDI
//
//  Created by abechan on 2/17/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MeasureScaleAssistant.h"
#import "TrackSelection.h"

@interface TrackViewController : NSViewController
@property (strong, nonatomic) MeasureScaleAssistant *scaleAssistant;
@property (strong, nonatomic) TrackSelection *trackSelection;
@end
