//
//  PlayLineViewController.h
//  NAMIDI
//
//  Created by abechan on 2/19/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MeasureScaleAssistant.h"
#import "NAMidiRepresentation.h"

@interface PlayLineViewController : NSViewController
@property (weak, nonatomic) NSView *containerView;
@property (strong, nonatomic) NAMidiRepresentation *namidi;
@property (strong, nonatomic) MeasureScaleAssistant *scaleAssistant;
@end
