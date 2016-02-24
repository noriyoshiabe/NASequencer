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

- (BOOL)isAvailable:(int)trackNo
{
    if (0 == trackNo) {
        return 0 < _namidi.sequence.eventsOfConductorTrack.count;
    }
    else {
        return _namidi.sequence.channels[trackNo - 1].exist;
    }
}

- (BOOL)isTrackSelected:(int)trackNo
{
    return [self isAvailable:trackNo] && 0 != (_selectionFlags & (1 << trackNo));
}

- (BOOL)isTrackSelectionChanged:(int)trackNo
{
    return (_previousSelectionFlags & (1 << trackNo)) != (_selectionFlags & (1 << trackNo));
}

- (void)setSelectionFlags:(unsigned int)selectionFlags
{
    _previousSelectionFlags = _selectionFlags;
    _selectionFlags = selectionFlags;
}

- (void)click:(int)trackNo event:(NSEvent *)event
{
    if (0 <= trackNo && trackNo <= 16) {
        if (NSCommandKeyMask & [NSEvent modifierFlags]) {
            [self toggle:trackNo];
        }
        else if (NSShiftKeyMask & [NSEvent modifierFlags]) {
            [self selectFromLastSelectedTo:trackNo];
        }
        else {
            switch (event.clickCount) {
                case 1:
                    if (![self isTrackSelected:trackNo]) {
                        [self deselectAll];
                        [self select:trackNo withUpdateLast:YES];
                    }
                    break;
                case 2:
                    [_delegate trackSelectionDidEnterSelection:self];
                    break;
            }
            
        }
    }
    else {
        [self deselectAll];
    }
}

- (void)select:(int)trackNo
{
    [self select:trackNo withUpdateLast:NO];
}

- (void)select:(int)trackNo withUpdateLast:(BOOL)updateLast
{
    self.selectionFlags = _selectionFlags | (1 << trackNo);
    if (updateLast) {
        _lastSelected = trackNo;
    }
}

- (void)selectFromLastSelectedTo:(int)trackNo
{
    unsigned int selectionFlags = _selectionFlags;
    
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
        [self select:trackNo withUpdateLast:YES];
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
