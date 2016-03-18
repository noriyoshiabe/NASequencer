//
//  SelectionViewController.h
//  NAMIDI
//
//  Created by abechan on 2/21/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TrackSelection.h"

@class SelectionViewController;
@protocol SelectionViewControllerDelegate <NSObject>
- (void)selectionViewControllerDidToggleListOpened:(SelectionViewController *)controller;
@end

@interface SelectionViewController : NSViewController
@property (weak, nonatomic) id<SelectionViewControllerDelegate> delegate;
@property (strong, nonatomic) NAMidiRepresentation *namidi;
@property (strong, nonatomic) TrackSelection *trackSelection;
@property (assign, nonatomic) BOOL listOpened;
- (IBAction)showEventList:(id)sender;
@end
