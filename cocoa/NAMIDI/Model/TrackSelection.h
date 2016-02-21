//
//  TrackSelection.h
//  NAMIDI
//
//  Created by abechan on 2/21/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface TrackSelection : NSObject
@property (readonly, nonatomic) unsigned int selectionFlags;
- (BOOL)isTrackSelected:(int)trackNo;
- (BOOL)isTrackSelectionChanged:(int)trackNo;
- (void)click:(int)trackNo;
- (void)selectAll;
- (void)deselectAll;
@end
