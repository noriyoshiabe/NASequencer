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

@class MeasureViewController;
@protocol MeasureViewControllerDelegate <NSObject>
- (void)measureViewControllerDidClickMeasure:(MeasureViewController *)controller tick:(int)tick;
- (void)measureViewControllerDidClickConductorTrack:(MeasureViewController *)controller;
@end

@interface MeasureViewController : NSViewController
@property (weak, nonatomic) id<MeasureViewControllerDelegate> delegate;
@property (strong, nonatomic) MeasureScaleAssistant *scaleAssistant;
@property (strong, nonatomic) TrackSelection *trackSelection;
@end
