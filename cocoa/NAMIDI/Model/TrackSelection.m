//
//  TrackSelection.m
//  NAMIDI
//
//  Created by abechan on 2/21/16.
//  Copyright © 2016 Noriyoshi Abe. All rights reserved.
//

#import "TrackSelection.h"

@interface TrackSelection () {
    unsigned int _previousSelectionFlags;
    int _lastSelected;
}

@end

@implementation TrackSelection

- (BOOL)isTrackSelected:(int)trackNo
{
    return 0 != (_selectionFlags & (1 << trackNo));
}

- (BOOL)isTrackSelectionChanged:(int)trackNo
{
    return (_previousSelectionFlags & (1 << trackNo)) != (_selectionFlags & (1 << trackNo));
}

- (void)setSelectionFlags:(unsigned int)selectionFlags
{
    _previousSelectionFlags = _selectionFlags;
    [self willChangeValueForKey:@"selectionFlags"];
    _selectionFlags = selectionFlags;
    [self didChangeValueForKey:@"selectionFlags"];
}

- (void)click:(int)trackNo
{
    if (0 <= trackNo && trackNo <= 16) {
        if (NSCommandKeyMask & [NSEvent modifierFlags]) {
            [self toggle:trackNo];
        }
        else if (NSShiftKeyMask & [NSEvent modifierFlags]) {
            [self selectFromLastSelectedTo:trackNo];
        }
        else {
            [self select:trackNo];
        }
    }
    else {
        [self deselectAll];
    }
}

- (void)select:(int)trackNo
{
    self.selectionFlags = (1 << trackNo);
    _lastSelected = trackNo;
}

- (void)selectFromLastSelectedTo:(int)trackNo
{
    unsigned int selectionFlags = 0;
    
    int from = _lastSelected < trackNo ? _lastSelected : trackNo;
    int to = _lastSelected < trackNo ? trackNo : _lastSelected;
    
    for (int i = from; i <= to; ++i) {
        selectionFlags |= (1 << i);
    }
    
    self.selectionFlags = selectionFlags;
}

- (void)deselect:(int)trackNo
{
    self.selectionFlags = _selectionFlags & ~(1 << trackNo);
}

- (void)toggle:(int)trackNo
{
    if ([self isTrackSelected:trackNo]) {
        [self deselect:trackNo];
    }
    else {
        [self select:trackNo];
    }
}

- (void)selectAll
{
    self.selectionFlags = 0x1FFFF;
    _lastSelected = 0;
}

- (void)deselectAll
{
    self.selectionFlags = 0;
    _lastSelected = 0;
}


@end
