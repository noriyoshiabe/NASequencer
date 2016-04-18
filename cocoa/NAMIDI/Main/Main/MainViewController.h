//
//  MainViewController.h
//  NAMIDI
//
//  Created by abechan on 2/9/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "NAMidiRepresentation.h"
#import "MeasureScaleAssistant.h"
#import "TrackSelection.h"

@class MainViewController;
@protocol MainViewControllerDelegate <NSObject>
- (void)mainViewControllerDidEnterSelection:(MainViewController *)controller;
@end

@interface MainViewController : NSViewController
@property (weak, nonatomic) id<MainViewControllerDelegate> delegate;
@property (strong, nonatomic) NAMidiRepresentation *namidi;
@property (strong, nonatomic) MeasureScaleAssistant *scaleAssistant;
@property (strong, nonatomic) TrackSelection *trackSelection;
@property (assign, nonatomic) CGPoint scrollPoint;
@end
