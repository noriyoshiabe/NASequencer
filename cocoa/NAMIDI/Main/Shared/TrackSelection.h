//
//  TrackSelection.h
//  NAMIDI
//
//  Created by abechan on 2/21/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@class TrackSelection;
@protocol TrackSelectionDelegate <NSObject>
- (void)trackSelectionDidEnterSelection:(TrackSelection *)trackSelection;
@end

@interface TrackSelection : NSObject
@property (weak, nonatomic) id<TrackSelectionDelegate> delegate;
@property (readonly, nonatomic) unsigned int selectionFlags;
- (BOOL)isTrackSelected:(int)trackNo;
- (BOOL)isTrackSelectionChanged:(int)trackNo;
- (void)click:(int)trackNo event:(NSEvent *)event;
- (void)select:(int)trackNo;
- (void)deselect:(int)trackNo;
- (void)selectAll;
- (void)deselectAll;
@end
